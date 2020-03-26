#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "SpectrogramComponent.h"

class DrumFixerAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    DrumFixerAudioProcessorEditor (DrumFixerAudioProcessor&);
    ~DrumFixerAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

private:
    DrumFixerAudioProcessor& processor;

    SpectrogramComponent specgram;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumFixerAudioProcessorEditor)
};
