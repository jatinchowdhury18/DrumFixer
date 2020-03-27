#ifndef RMSDETECTOR_H_INCLUDED
#define RMSDETECTOR_H_INCLUDED

#include "BaseDetector.h"

class RMSDetector : public BaseDetector
{
public:
    RMSDetector() {}

    inline float process (float x) override
    {
        auto input = x * x;
        auto outputSquare = 0.0f;

        if (input > levelEst)
            outputSquare = a1_a * levelEst + b0_a * input;
        else
            outputSquare = a1_r * levelEst + b0_r * input;

        levelEst = outputSquare;
        return sqrt (outputSquare);
    }

    void processBlock (float* buffer, int numSamples)
    {
        for (int n = 0; n < numSamples; ++n)
            buffer[n] = process (buffer[n]);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RMSDetector)
};

#endif // RMSDETECTOR_H_INCLUDED
