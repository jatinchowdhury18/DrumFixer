#ifndef LPF_H_INCLUDED
#define LPF_H_INCLUDED

#include "Biquad.h"

class LPF2 : public Biquad
{
public:
    LPF2() {}

    void calcCoefs (float newFc, float newQ, float newGainDB = 0.0f) override
    {
        fc = newFc;
        Q = newQ;
        gainDB = newGainDB;

        float wc = MathConstants<float>::twoPi * fc / fs;
        float c = 1.0f / tan (wc / 2.0f);
        float phi = c * c;
        float K = c / newQ;
        float a0 = phi + K + 1.0f;

        b[0] = 1.0f / a0;
        b[1] = 2.0f * b[0];
        b[2] = b[0];
        a[1] = 2.0f * (1.0f - phi) / a0;
        a[2] = (phi - K + 1.0f) / a0;
    }

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LPF2)
};

#endif // LPF_H_INCLUDED
