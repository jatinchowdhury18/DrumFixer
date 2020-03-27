#ifndef BELL_H_INCLUDED
#define BELL_H_INCLUDED

#include "Biquad.h"

class BellFilter : public Biquad
{
public:
    BellFilter() {}

    void calcCoefs (float newFc, float newQ, float newGainDB = 0.0f) override
    {
        fc = newFc;
        Q = newQ;
        gainDB = newGainDB;

        float wc = MathConstants<float>::twoPi * fc / fs;
        float c = 1.0f / tan (wc / 2.0f);
        float phi = c * c;
        float Knum = c / Q;
        float Kdenom = Knum;

        if (gainDB > 0.0f)
            Knum *= Decibels::decibelsToGain (gainDB);
        else if (gainDB < 0.0f)
            Kdenom /= Decibels::decibelsToGain (gainDB);

        float a0 = phi + Kdenom + 1.0f;

        b[0] = (phi + Knum + 1.0f) / a0;
        b[1] = 2.0f * (1.0f - phi) / a0;
        b[2] = (phi - Knum + 1.0f) / a0;

        a[1] = 2.0f * (1.0f - phi) / a0;
        a[2] = (phi - Kdenom + 1.0f) / a0;
    }

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BellFilter)
};

#endif // BELL_H_INCLUDED
