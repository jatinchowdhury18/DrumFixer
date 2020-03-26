#include "FFTUtils.h"

FFTUtils::FFTUtils() :
    forwardFFT (fftOrder)
{
}

void FFTUtils::processBlock (AudioBuffer<float>& buffer)
{
    auto data = buffer.getArrayOfReadPointers();
    for (int n = 0; n < buffer.getNumSamples(); ++n)
    {
        float sample = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            sample += abs (data[ch][n]);

        pushNextSampleIntoFifo (sample / (float) buffer.getNumChannels());
    }
}

void FFTUtils::pushNextSampleIntoFifo (float sample) noexcept
{
    if (fifoIndex == fftSize)
    {
        if (! nextFFTBlockReady)
        {
            zeromem (fftData, sizeof (fftData));
            memcpy (fftData, fifo, sizeof (fifo));
            nextFFTBlockReady = true;
        }

        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

void FFTUtils::drawNextLineOfSpectrogram (Image& spectrogramImage)
{
    auto rightHandEdge = spectrogramImage.getWidth() - 1;
    auto imageHeight   = spectrogramImage.getHeight();

    // first, shuffle our image leftwards by 1 pixel..
    spectrogramImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);

    // then render our FFT data..
    forwardFFT.performFrequencyOnlyForwardTransform (fftData);
    const auto max = FloatVectorOperations::findMaximum (fftData, fftSize/2);
    const float minDB = -100.0f;
    for (int k = 0; k < fftSize/2; ++k)
        fftDataDB[k] = Decibels::gainToDecibels (fftData[k] / max, minDB);

    for (auto y = 1; y < imageHeight; ++y)
    {
        auto skewedProportionY = 1.0f - std::exp (std::log (y / (float) imageHeight) * 0.2f);
        auto fftDataIndex = jlimit (0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));
        auto level = jmap (fftDataDB[fftDataIndex], minDB, 0.0f, 0.0f, 1.0f);

        spectrogramImage.setPixelAt (rightHandEdge, y, Colour::fromHSV (level, 1.0f, level, 1.0f));
    }

    nextFFTBlockReady = false;
}
