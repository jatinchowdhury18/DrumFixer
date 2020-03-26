#ifndef BASEDETECTOR_H_INCLUDED
#define BASEDETECTOR_H_INCLUDED

#include "JuceHeader.h"

class BaseDetector
{
public:
    BaseDetector() {}
    virtual ~BaseDetector() {}

    virtual void setAttackMs  (float newAttackMs);
    virtual void setReleaseMs (float newReleaseMs);

    void reset (float sampleRate);
    virtual inline float process (float /*x*/) = 0;

protected:
    // Attack coefs
    float attackMs = 1.0f;
    float a1_a = 0.0f;
    float b0_a = 1.0f;

    //  Release coefs
    float releaseMs = 20.0f;
    float a1_r = 0.0f;
    float b0_r = 1.0f;

    float levelEst = 0.0f;

private:
    float fs = 48000.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BaseDetector)
};

#endif // BASEDETECTOR_H_INCLUDED
