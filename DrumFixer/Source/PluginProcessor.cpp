#include "PluginProcessor.h"
#include "PluginEditor.h"

DrumFixerAudioProcessor::DrumFixerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

DrumFixerAudioProcessor::~DrumFixerAudioProcessor()
{
}

const String DrumFixerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DrumFixerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DrumFixerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DrumFixerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DrumFixerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DrumFixerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DrumFixerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DrumFixerAudioProcessor::setCurrentProgram (int index)
{
}

const String DrumFixerAudioProcessor::getProgramName (int index)
{
    return {};
}

void DrumFixerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

void DrumFixerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fftUtils.setSampleRate (getSampleRate());
    tDetect.prepare (sampleRate, getMainBusNumInputChannels(), samplesPerBlock);

    for (auto filt : decayFilts)
        filt->prepare (sampleRate, samplesPerBlock);
}

void DrumFixerAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DrumFixerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DrumFixerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    
    if (listening)
    {
        if (tDetect.isTransientInBuffer (buffer))
        {
            transDetected = true;
            fftUtils.processBlock (buffer);

            auto curTransBufferSize = transientBuffer.getNumSamples();
            transientBuffer.setSize (buffer.getNumChannels(),
                curTransBufferSize + buffer.getNumSamples(), true);
            
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                transientBuffer.copyFrom (ch, curTransBufferSize, buffer, ch, 0, buffer.getNumSamples());
        }
        else if (transDetected)
        {
            toggleListening();
        }
    }
    else
    {
        int transSample = tDetect.isTransientStarting (buffer);

        if (transSample >= 0)
        {
            for (auto filt : decayFilts)
                filt->newTransient (transSample);
        }

        for (auto filt : decayFilts)
            filt->processBlock (buffer);
    }
}

void DrumFixerAudioProcessor::toggleListening()
{
    if (! listening) // start listening
    {
        listening = true;
        transDetected = false;
        
        transientBuffer.clear();
        transientBuffer.setSize (getMainBusNumInputChannels(), 0);

        tDetect.resetTransient();
    }
    else // stop listening
    {
        listening = false;
        transientSampleRate = getSampleRate();

        for (auto* filt : decayFilts)
            filt->updateFilter();
    }

    sendChangeMessage();
}

void DrumFixerAudioProcessor::addDecayFilter (DecayFilter::Params& params)
{
    if (transientBuffer.hasBeenCleared())
        return;

    decayFilts.add (new DecayFilter (params, transientBuffer, transientSampleRate));
    decayFilts.getLast()->prepare (getSampleRate(), getBlockSize());
}

bool DrumFixerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DrumFixerAudioProcessor::createEditor()
{
    return new DrumFixerAudioProcessorEditor (*this);
}

void DrumFixerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // create root xml element
    auto xmlState = std::make_unique<XmlElement> ("PluginState");

    // save spectrogram image to file
    if (auto editor = dynamic_cast<DrumFixerAudioProcessorEditor*> (getActiveEditor()))
        editor->getSpectrogramImage();

    if (specgramImage.isValid())
    {
        File specgramFile = File::getSpecialLocation (File::userApplicationDataDirectory)
            .getChildFile ("DrumFixer")
            .getChildFile ("Spectrograms")
            .getNonexistentChildFile ("specgram", ".png", false);
        specgramFile.create();

        FileOutputStream fStream (specgramFile);
        PNGImageFormat png;
        png.writeImageToStream (specgramImage, fStream);
        xmlState->setAttribute ("Spectrogram", specgramFile.getFullPathName());
    }

    // save transient buffer to file
    if (! transientBuffer.hasBeenCleared() && transientBuffer.getNumChannels() > 0)
    {
        File transientFile = File::getSpecialLocation (File::userApplicationDataDirectory)
            .getChildFile ("DrumFixer")
            .getChildFile ("Transients")
            .getNonexistentChildFile ("transient", ".wav", false);
        transientFile.create();

        WavAudioFormat wav;
        std::unique_ptr<AudioFormatWriter> writer (wav
            .createWriterFor (new FileOutputStream (transientFile), transientSampleRate,
                              transientBuffer.getNumChannels(), 16, {}, 0));
        writer->writeFromAudioSampleBuffer (transientBuffer, 0, transientBuffer.getNumSamples());
        xmlState->setAttribute ("Transient", transientFile.getFullPathName());
    }

    // save decay filters
    auto filtersXml = std::make_unique<XmlElement> ("Filters");
    int i = 0;
    for (auto filt : decayFilts)
    {
        auto filtXml = filt->toXml();
        filtXml->setTagName ("DecayFilter" + String (i++));
        filtersXml->addChildElement (filtXml.release());
    }

    xmlState->addChildElement (filtersXml.release());

    // write XML to file
    copyXmlToBinary (*xmlState.get(), destData);
}

void DrumFixerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // read XML from file
    auto xmlState = getXmlFromBinary (data, sizeInBytes);
    
    // read spectrogram image
    if (xmlState->hasAttribute ("Spectrogram"))
    {
        auto specgramFile = File (xmlState->getStringAttribute ("Spectrogram"));
        specgramImage = PNGImageFormat::loadFrom (specgramFile);
    }

    // read transient file
    if (xmlState->hasAttribute ("Transient"))
    {
        transientSampleRate = xmlState->getDoubleAttribute ("SampleRate");

        auto transientFile = File (xmlState->getStringAttribute ("Transient"));
        WavAudioFormat wav;
        std::unique_ptr<AudioFormatReader> reader (wav
            .createReaderFor (new FileInputStream (transientFile), true));

        transientBuffer.setSize (reader->numChannels, reader->lengthInSamples);
        reader->read (&transientBuffer, 0, reader->lengthInSamples, 0, true, true);
        transientSampleRate = reader->sampleRate;
    }

    // load decay filters
    decayFilts.clear();
    if (auto filtersXml = xmlState->getChildByName ("Filters"))
    {
        forEachXmlChildElement (*filtersXml, filtXml)
            decayFilts.add (DecayFilter::fromXml (filtXml, transientBuffer, transientSampleRate));
    }

    if (auto editor = dynamic_cast<DrumFixerAudioProcessorEditor*> (getActiveEditor()))
        editor->reload();
}

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumFixerAudioProcessor();
}
