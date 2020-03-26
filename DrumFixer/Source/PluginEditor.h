#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "SpectrogramComponent.h"

class DrumFixerAudioProcessorEditor : public AudioProcessorEditor,
                                      private ChangeListener
{
public:
    DrumFixerAudioProcessorEditor (DrumFixerAudioProcessor&);
    ~DrumFixerAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

    void listenButtonPressed();

    void changeListenerCallback (ChangeBroadcaster* source);

private:
    DrumFixerAudioProcessor& processor;

    SpectrogramComponent specgram;

    TextButton listenButton { "Listen" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumFixerAudioProcessorEditor)
};
