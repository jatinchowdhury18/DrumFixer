#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DSP/FFTUtils.h"
#include "DSP/LevelDetectors/TransientDetector.h"
#include "DSP/DecayFilter.h"

class DrumFixerAudioProcessor : public AudioProcessor,
                                public ChangeBroadcaster
{
public:
    DrumFixerAudioProcessor();
    ~DrumFixerAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    FFTUtils& getFFT() {  return fftUtils; }

    bool isListening() { return listening; }
    void toggleListening();

    void addDecayFilter (DecayFilter::Params& params);
    OwnedArray<DecayFilter>& getDecayFilters() { return decayFilts; }

private:
    FFTUtils fftUtils;

    TransientDetector tDetect;
    bool listening = false;
    bool transDetected = false;

    OwnedArray<DecayFilter> decayFilts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumFixerAudioProcessor)
};
