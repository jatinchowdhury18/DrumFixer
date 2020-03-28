#include "TransientDetector.h"

void TransientDetector::prepare (double sampleRate, int nChannels, int nSamples)
{
    for (int ch = 0; ch < 2; ++ch)
    {
        pDetect[ch].reset ((float) sampleRate);
        pDetect[ch].setAttackMs (0.1f);
        pDetect[ch].setReleaseMs (150.0f);
    }

    detectBuffer.setSize (nChannels, nSamples);
}

void TransientDetector::resetTransient()
{ 
    count = countToWait;
    noiseFloor = Decibels::decibelsToGain (0.0f);
}

int TransientDetector::isTransientStarting (const AudioBuffer<float>& buffer)
{
    if (! isTransientInBuffer (buffer))
    {
        inTransient = false;
        return -1;
    }

    if (inTransient)
        return -1;

    inTransient = true;

    // find argmax
    auto getMaxValueAtIndex = [=] (int idx)
    {
        float value = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto newVal = abs (detectBuffer.getSample (ch, idx));
            if (newVal > value)
                value = newVal;
        }

        return value;
    };

    int maxSample = 0;
    float maxVal = getMaxValueAtIndex (maxSample);
    for (int n = 1; n < buffer.getNumSamples(); ++n)
    {
        auto newMaxVal = getMaxValueAtIndex (n);
        if (newMaxVal > maxVal)
        {
            maxVal = newMaxVal;
            maxSample = n;
        }
    }

    return maxSample;
}

bool TransientDetector::isTransientInBuffer (const AudioBuffer<float>& buffer)
{
    processDetectBuffer (buffer);

    if (detectBuffer.getMagnitude (0, buffer.getNumSamples()) > 5*noiseFloor)
    {
        count = 0;
        return true;
    }

    // update noise floor (moving avg of rms values)
    float rms = 0.01f;
    for (int ch = 0; ch < detectBuffer.getNumChannels(); ++ch)
        rms = jmax (rms, detectBuffer.getRMSLevel (ch, 0, buffer.getNumSamples()));

    noiseFloor -= noiseFloor / (float) movingAvgN;
    noiseFloor += rms / (float) movingAvgN;

    // if still in wait buffer, return true
    if (count < countToWait)
    {
        count += buffer.getNumSamples();
        return true;
    }

    return false;
}

void TransientDetector::processDetectBuffer (const AudioBuffer<float>& buffer)
{
    detectBuffer.makeCopyOf (buffer, true);

    for (int ch = 0; ch < detectBuffer.getNumChannels(); ++ch)
    {
        auto x = detectBuffer.getWritePointer (ch);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
            x[n] = pDetect[ch].process (x[n]);
    }
}
