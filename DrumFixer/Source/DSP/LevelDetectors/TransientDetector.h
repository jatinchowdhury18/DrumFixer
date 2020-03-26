#ifndef TRANSIENTDETECTOR_H_INCLUDED
#define TRANSIENTDETECTOR_H_INCLUDED

#include "PeakDetector.h"

class TransientDetector
{
public:
    TransientDetector() {}

    void prepare (double sampleRate, int nChannels, int nSamples)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            pDetect[ch].reset ((float) sampleRate);
            pDetect[ch].setAttackMs (0.1f);
            pDetect[ch].setReleaseMs (150.0f);
        }

        detectBuffer.setSize (nChannels, nSamples);
    }

    bool isTransientInBuffer (const AudioBuffer<float>& buffer)
    {
        detectBuffer.makeCopyOf (buffer, true);

        for (int ch = 0; ch < detectBuffer.getNumChannels(); ++ch)
        {
            auto x = detectBuffer.getWritePointer (ch);
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                x[n] = pDetect[ch].process (x[n]);
        }

        if (detectBuffer.getMagnitude (0, buffer.getNumSamples()) > 5*noiseFloor)
            return true;

        // update noise floor (moving avg of rms values)
        float rms = 0.01f;
        for (int ch = 0; ch < detectBuffer.getNumChannels(); ++ch)
            rms = jmax (rms, detectBuffer.getRMSLevel (ch, 0, buffer.getNumSamples()));

        noiseFloor -= noiseFloor / (float) movingAvgN;
        noiseFloor += rms / (float) movingAvgN;

        return false;
    }

private:
    AudioBuffer<float> detectBuffer;
    PeakDetector pDetect[2];

    float noiseFloor = Decibels::decibelsToGain (0.0f);
    const int movingAvgN = 20;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransientDetector)
};

#endif // TRANSIENTDETECTOR_H_INCLUDED
