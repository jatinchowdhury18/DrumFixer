#include "PluginProcessor.h"
#include "PluginEditor.h"

DrumFixerAudioProcessorEditor::DrumFixerAudioProcessorEditor (DrumFixerAudioProcessor& p) :
    AudioProcessorEditor (&p),
    processor (p),
    specgram (p.getFFT()),
    overlay (p),
    filterList (p)
{
    setSize (650, 450);
    
    processor.addChangeListener (this);

    addAndMakeVisible (specgram);

    addAndMakeVisible (overlay);
    overlay.setMouseCursor (MouseCursor::NoCursor);
    overlay.addChangeListener (this);

    addAndMakeVisible (filterList);
    filterList.addChangeListener (this);

    addAndMakeVisible (listenButton);
    listenButton.onClick = [=] { listenButtonPressed(); };

    reload();
}

DrumFixerAudioProcessorEditor::~DrumFixerAudioProcessorEditor()
{
    getSpectrogramImage();
}

void DrumFixerAudioProcessorEditor::reload()
{
    specgram.setImage (processor.specgramImage);
    filterList.updateContent();
    repaint();
}

void DrumFixerAudioProcessorEditor::getSpectrogramImage()
{
    processor.specgramImage = specgram.getImageCopy();
}

void DrumFixerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}

void DrumFixerAudioProcessorEditor::resized()
{
    const int pad = 10;
    specgram.setBounds (pad, pad, 400 - 2*pad, getHeight() - 2*pad);
    overlay.setBounds (specgram.getBounds());

    listenButton.setBounds (410, 10, 100, 30);

    filterList.setBounds (400, 50, 240, getHeight() - 50 - 2*pad);
    filterList.setRowHeight (30);
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

    if (source == &overlay)
    {
        filterList.updateContent();
    }

    if (source == &filterList)
    {
        overlay.repaint();
    }
}
