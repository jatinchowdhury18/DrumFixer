#include "PluginProcessor.h"
#include "PluginEditor.h"

DrumFixerAudioProcessorEditor::DrumFixerAudioProcessorEditor (DrumFixerAudioProcessor& p) :
    AudioProcessorEditor (&p),
    processor (p),
    specgram (p.getFFT())
{
    setSize (400, 400);
    
    addAndMakeVisible (specgram);
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
    specgram.setBounds (getLocalBounds());
}
