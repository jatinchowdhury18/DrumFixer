#include "PluginProcessor.h"
#include "PluginEditor.h"

DrumFixerAudioProcessorEditor::DrumFixerAudioProcessorEditor (DrumFixerAudioProcessor& p) :
    AudioProcessorEditor (&p),
    processor (p),
    specgram (p.getFFT()),
    overlay (p)
{
    setSize (800, 450);
    
    processor.addChangeListener (this);

    addAndMakeVisible (specgram);
    addAndMakeVisible (overlay);
    overlay.setMouseCursor (MouseCursor::NoCursor);

    addAndMakeVisible (listenButton);
    listenButton.onClick = [=] { listenButtonPressed(); };
}

DrumFixerAudioProcessorEditor::~DrumFixerAudioProcessorEditor()
{
}

void DrumFixerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}

void DrumFixerAudioProcessorEditor::resized()
{
    const int pad = 10;
    specgram.setBounds (pad, pad, getWidth() / 2 - 2*pad, getHeight() - 2*pad);
    overlay.setBounds (specgram.getBounds());

    listenButton.setBounds (410, 10, 100, 30);
}

void DrumFixerAudioProcessorEditor::listenButtonPressed()
{
    processor.toggleListening();
}

void DrumFixerAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &processor)
    {
        if (processor.isListening()) // started listening
        {
            specgram.clear();
            listenButton.setButtonText ("Stop");
        }
        else // stopped listening
        {
            specgram.drawSpecgram();
            listenButton.setButtonText ("Listen");
        }
    }
}
