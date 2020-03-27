#include "ModeBandpass.h"

namespace
{
    constexpr float butterQs[2] = { 0.5412f, 1.3065f };
}

ModeBandpass::ModeBandpass (float fc, float bandwidth, double fs)
{
    float highFreq = jmin (fc + bandwidth / 2.0f, (float) fs / 2.0f - 50.0f);
    float lowFreq = jmax (fc - bandwidth / 2.0f, 10.0f);

    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < 2; ++i)
        {
            lpfs[ch][i].reset (fs);
            lpfs[ch][i].calcCoefs (highFreq, butterQs[i]);

            hpfs[ch][i].reset (fs);
            hpfs[ch][i].calcCoefs (lowFreq, butterQs[i]);
        }
    }
}

void ModeBandpass::processBlock (AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* x = buffer.getWritePointer (ch);

        for (int i = 0; i < 2; ++i)
        {
            lpfs[ch][i].processBlock (x, buffer.getNumSamples());
            hpfs[ch][i].processBlock (x, buffer.getNumSamples());
        }
    }
}
