#ifndef MODEBANDPASS_H_INCLUDED
#define MODEBANDPASS_H_INCLUDED

#include "LPF.h"
#include "HPF.h"

/** 
    4th order Butterworth BPF
    Centered around a mode, with some desired bandwidth
*/
class ModeBandpass
{
public:
    ModeBandpass (float fc, float bandwidth, double fs);

    void processBlock (AudioBuffer<float>& buffer);

private:
    LPF2 lpfs[2][2];
    HPF2 hpfs[2][2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModeBandpass)
};

#endif // MODEBANDPASS_H_INCLUDED
