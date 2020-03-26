#ifndef SPECTROGRAMCOMPONENT_H_INCLUDED
#define SPECTROGRAMCOMPONENT_H_INCLUDED

#include "DSP/FFTUtils.h"

class SpectrogramComponent : public Component,
                             private Timer
{
public:
    SpectrogramComponent (FFTUtils& fft);

    void paint (Graphics& g) override;
    void clear();
    void drawSpecgram();

private:
    void timerCallback() override;

    enum
    {
        specgramPixels = 512,
    };

    FFTUtils& fft;
    std::unique_ptr<Image> spectrogramImage;
    Array<Array<Colour>> specgramData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramComponent)
};

#endif // SPECTROGRAMCOMPONENT_H_INCLUDED
