#ifndef TRANSIENTDETECTOR_H_INCLUDED
#define TRANSIENTDETECTOR_H_INCLUDED

#include "PeakDetector.h"

class TransientDetector
{
public:
    TransientDetector() {}

    void prepare (double sampleRate, int nChannels, int nSamples);
    void resetTransient();

    int isTransientStarting (const AudioBuffer<float>& buffer);
    bool isTransientInBuffer (const AudioBuffer<float>& buffer);

    void processDetectBuffer (const AudioBuffer<float>& buffer);

private:
    AudioBuffer<float> detectBuffer;
    PeakDetector pDetect[2];

    float noiseFloor = Decibels::decibelsToGain (0.0f);
    const int movingAvgN = 20;

    const int countToWait = 2048;
    int count = countToWait;

    bool inTransient = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransientDetector)
};

#endif // TRANSIENTDETECTOR_H_INCLUDED
