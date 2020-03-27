#ifndef DECAYFILTER_H_INCLUDED
#define DECAYFILTER_H_INCLUDED

#include "JuceHeader.h"

class DecayFilter
{
public:
    struct Params
    {
        Params (float fc, float bw, float tau) :
            centerFreq (fc),
            bandwidth (bw),
            desiredT60 (tau)
        {}

        Params (Params& p) :
            centerFreq (p.centerFreq),
            bandwidth (p.bandwidth),
            desiredT60 (p.desiredT60)
        {}

        float centerFreq;
        float bandwidth;
        float desiredT60;
    };

    DecayFilter (Params& params);

    Params& getParams() { return params; }

private:
    Params params;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecayFilter)
};

#endif // DECAYFILTER_H_INCLUDED
