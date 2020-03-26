#include "SpectrogramComponent.h"

SpectrogramComponent::SpectrogramComponent (FFTUtils& fft) :
    fft (fft),
    spectrogramImage (Image::RGB, 512, 512, true)
{
    setOpaque (true);
    startTimerHz (45);
}

void SpectrogramComponent::timerCallback()
{
    if (fft.isFFTReady())
    {
        fft.drawNextLineOfSpectrogram (spectrogramImage);
        repaint();
    }
}

void SpectrogramComponent::paint (Graphics& g)
{
    g.fillAll (Colours::black);

    g.setOpacity (1.0f);
    g.drawImage (spectrogramImage, getLocalBounds().toFloat());
}
