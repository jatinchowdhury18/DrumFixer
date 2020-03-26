#include "BaseDetector.h"

void BaseDetector::setAttackMs (float newAttackMs)
{
    if (attackMs != newAttackMs)
    {
        attackMs = newAttackMs;
        a1_a = exp (-1.0f / (fs * attackMs / 1000.0f));
        b0_a = 1.0f - a1_a;
    }
}

void BaseDetector::setReleaseMs (float newReleaseMs)
{
    if (releaseMs != newReleaseMs)
    {
        releaseMs = newReleaseMs;
        a1_r = exp (-1.0f / (fs * releaseMs / 1000.0f));
        b0_r = 1.0f - a1_r;
    }
}

void BaseDetector::reset (float sampleRate)
{
    fs = sampleRate;
    levelEst = 0.0f;

    setAttackMs (attackMs);
    setReleaseMs (releaseMs);
}
