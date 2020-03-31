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

    auto& fftUtils = processor.getFFT();
    g.setGradientFill (ColourGradient (fftUtils.getInfernoColour (0.88f), 5, 5,
        fftUtils.getInfernoColour (0.25f), 250, 50, false));

    GlyphArrangement ga;
    ga.addLineOfText (Font (45.0f).boldened(), "DrumFixer", 5, 40);
    Path p;
    ga.createPath (p);
    g.fillPath (p);
}

void DrumFixerAudioProcessorEditor::resized()
{
    const int pad = 10;
    filterList.setBounds (pad, 100, 240, getHeight() - 50 - 2*pad);
    filterList.setRowHeight (30);

    listenButton.setBounds (75, 60, 100, 30);

    specgram.setBounds (260, pad, 400 - 2*pad, getHeight() - 2*pad);
    overlay.setBounds (specgram.getBounds());
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
