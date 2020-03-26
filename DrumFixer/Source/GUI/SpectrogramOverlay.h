#ifndef SPECTROGRAMOVERLAY_H_INCLUDED
#define SPECTROGRAMOVERLAY_H_INCLUDED

#include "JuceHeader.h"

class SpectrogramOverlay : public Component
{
public:
    SpectrogramOverlay();

    void paint (Graphics& g) override;

    void mouseMove (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    void mouseExit (const MouseEvent& e) override;

    void resetLabel();

private:
    Label cursorLabel;
    float mouseY = -1.0f;
    float mouseX = -1.0f;

    float freqY;
    float widthY;

    enum State
    {
        ChooseFreq,
        ChooseWidth,
        ChooseTau
    };

    State state = ChooseFreq;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramOverlay)
};

#endif // SPECTROGRAMOVERLAY_H_INCLUDED
