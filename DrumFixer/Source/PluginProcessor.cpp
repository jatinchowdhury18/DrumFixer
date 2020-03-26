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
        }
        else if (transDetected)
        {
            toggleListening();
        }
    }

    buffer.clear();
}

void DrumFixerAudioProcessor::toggleListening()
{
    if (! listening) // start listening
    {
        listening = true;
        transDetected = false;
    }
    else // stop listening
    {
        listening = false;
    }

    sendChangeMessage();
}

void DrumFixerAudioProcessor::addDecayFilter (DecayFilter::Params& params)
{
    decayFilts.add (new DecayFilter (params));
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DrumFixerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumFixerAudioProcessor();
}
