#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "GUI/SpectrogramComponent.h"
#include "GUI/SpectrogramOverlay.h"
#include "GUI/FilterList.h"

class DrumFixerAudioProcessorEditor : public AudioProcessorEditor,
                                      private ChangeListener
{
public:
    DrumFixerAudioProcessorEditor (DrumFixerAudioProcessor&);
    ~DrumFixerAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

    void reload();
    void getSpectrogramImage();

    void listenButtonPressed();

    void changeListenerCallback (ChangeBroadcaster* source);

private:
    DrumFixerAudioProcessor& processor;

    SpectrogramComponent specgram;
    SpectrogramOverlay overlay;
    FilterList filterList;

    TextButton listenButton { "Listen" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumFixerAudioProcessorEditor)
};
