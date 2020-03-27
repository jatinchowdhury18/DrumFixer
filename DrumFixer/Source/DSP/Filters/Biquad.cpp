#include "Biquad.h"

void Biquad::reset (double sampleRate)
{
    // clear filter state
    for (int n = 0; n <= order; ++n)
        z[n] = 0.0f;

    fs = (float) sampleRate;

    calcCoefs (fc, Q, gainDB);
}

void Biquad::processBlock (float* buffer, int numSamples)
{
    for (int n = 0; n < numSamples; ++n)
        buffer[n] = process (buffer[n]);
}
