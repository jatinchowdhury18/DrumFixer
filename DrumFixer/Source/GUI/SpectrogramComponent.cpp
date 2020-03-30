#include "SpectrogramComponent.h"

SpectrogramComponent::SpectrogramComponent (FFTUtils& fft) :
    fft (fft)
{
    setOpaque (true);
    startTimerHz (45);
}

void SpectrogramComponent::timerCallback()
{
    if (fft.isFFTReady())
    {
        Array<Colour> spectrogramLine;
        fft.drawNextLineOfSpectrogram (spectrogramLine, specgramPixels);
        specgramData.add (spectrogramLine);
    }
}

void SpectrogramComponent::clear()
{
    spectrogramImage.reset (nullptr);
    specgramData.clear();
    repaint();
}

void SpectrogramComponent::drawSpecgram()
{
    if (specgramData.size() == 0)
        return;

    spectrogramImage = std::make_unique<Image> (Image::RGB, specgramData.size(), specgramPixels, true);
    for (int x = 0; x < specgramData.size(); ++x)
        for (int y = 0; y < specgramPixels; ++y)
            spectrogramImage->setPixelAt (x, y, specgramData[x][y]);

    repaint();
}

void SpectrogramComponent::paint (Graphics& g)
{
    g.fillAll (Colours::black);

    g.setOpacity (1.0f);
    if (spectrogramImage.get() != nullptr)
        g.drawImage (*spectrogramImage.get(), getLocalBounds().toFloat());
}

Image SpectrogramComponent::getImageCopy()
{
    if (spectrogramImage.get() == nullptr)
        return Image();

    return spectrogramImage->createCopy();
}

void SpectrogramComponent::setImage (Image image)
{
    if (image.isValid())
        spectrogramImage = std::make_unique<Image> (image);
}
