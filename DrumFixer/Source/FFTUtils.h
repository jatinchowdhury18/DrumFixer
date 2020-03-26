#ifndef FFTUTILS_H_INCLUDED
#define FFTUTILS_H_INCLUDED

#include "JuceHeader.h"

class FFTUtils
{
public:
    FFTUtils();

    void processBlock (AudioBuffer<float>& buffer);
    void pushNextSampleIntoFifo (float sample) noexcept;

    bool isFFTReady() { return nextFFTBlockReady; }
    void drawNextLineOfSpectrogram (Image& spectrogramImage);

private:
    enum
    {
        fftOrder = 10,
        fftSize  = 1 << fftOrder
    };

    dsp::FFT forwardFFT;

    float fifo [fftSize];
    float fftData [2 * fftSize];
    float fftDataDB [2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFTUtils)
};

#endif // FFTUTILS_H_INCLUDED
