#ifndef PEAKDETECTOR_H_INCLUDED
#define PEAKDETECTOR_H_INCLUDED

#include "BaseDetector.h"

class PeakDetector : public BaseDetector
{
public:
    PeakDetector() {}

    inline float process (float x) override
    {
        auto xAbs = abs (x);
        if (xAbs > levelEst)
            levelEst += b0_a * (xAbs - levelEst);
        else
            levelEst += b0_r * (xAbs - levelEst);
        
        return levelEst;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeakDetector)
};

#endif // PEAKDETECTOR_H_INCLUDED
