#ifndef BIQUAD_H_INCLUDED
#define BIQUAD_H_INCLUDED

#include "JuceHeader.h"

class Biquad
{
public:
    Biquad() {}
    virtual ~Biquad() {}

    virtual void calcCoefs (float /*fc*/, float /*Q*/, float /*gainDB*/) = 0;

    virtual void reset (double sampleRate);
    virtual void processBlock (float* buffer, int numSamples);

    virtual inline float process (float x)
    {
        // Direct Form II transposed
        float y = z[1] + x * b[0];
        z[1] = z[2] + x*b[1] - y*a[1];
        z[2] = x*b[2] - y*a[2];

        return y;
    }

protected:
    float fs = 48000.0f;

    float a[3] = { 1.0f, 0.0f, 0.0f };
    float b[3] = { 1.0f, 0.0f, 0.0f };
    float z[3] = { 1.0f, 0.0f, 0.0f };

    float fc = 1000.0f;
    float Q = 0.7071f;
    float gainDB = 0.0f;

private:
    const int order = 2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Biquad)
};

#endif // BIQUAD_H_INCLUDED
