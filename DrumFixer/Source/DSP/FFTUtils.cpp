#include "FFTUtils.h"

namespace
{
    constexpr float lowFreq = 40.0f;
    constexpr float highFreq = 20000.0f;
};

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
    const float minDB = -100.0f;
    for (int k = 0; k < fftSize/2; ++k)
        fftDataDB[k] = Decibels::gainToDecibels (fftData[k] / 200, minDB);

    const float binWidth = sampleRate / (float) fftSize;

    for (auto y = 0; y < imageHeight; ++y)
    {
        const auto freq = yToFreq ((float) y, (float) imageHeight);
        const auto fftIdx = jmin (freq / binWidth, (float) fftSize / 2);
        auto lower = fftDataDB[int (floor (fftIdx))];
        auto upper = fftDataDB[int (ceil (fftIdx))];
        const auto frac = fftIdx - floor (fftIdx);
        auto interp = upper * frac + lower * (1.0f - frac);

        auto level = jmap (interp, minDB, 0.0f, 0.0f, 1.0f);
        auto colour = inferno[int (level * 256)];

        spectrogramImage.setPixelAt (rightHandEdge, y,
            Colour::fromFloatRGBA (colour[0], colour[1], colour[2], 1.0f));
    }

    nextFFTBlockReady = false;
}

float FFTUtils::yToFreq (float y, float height)
{
    return lowFreq * pow ((highFreq / lowFreq), 1.0f - (y / height));
}

int FFTUtils::freqToY (float freq, float height)
{
    return int ((1.0f - (log (freq / lowFreq) / log (highFreq / lowFreq))) * height);
}