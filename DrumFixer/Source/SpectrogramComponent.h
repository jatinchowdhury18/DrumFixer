#ifndef SPECTROGRAMCOMPONENT_H_INCLUDED
#define SPECTROGRAMCOMPONENT_H_INCLUDED

#include "FFTUtils.h"

class SpectrogramComponent : public Component,
                             private Timer
{
public:
    SpectrogramComponent (FFTUtils& fft);

    void paint (Graphics& g) override;

private:
    void timerCallback() override;

    FFTUtils& fft;
    Image spectrogramImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramComponent)
};

#endif // SPECTROGRAMCOMPONENT_H_INCLUDED
