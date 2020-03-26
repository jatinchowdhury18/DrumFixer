#ifndef SPECTROGRAMOVERLAY_H_INCLUDED
#define SPECTROGRAMOVERLAY_H_INCLUDED

#include "../DSP/DecayFilter.h"
#include "../PluginProcessor.h"

class SpectrogramOverlay : public Component
{
public:
    SpectrogramOverlay (DrumFixerAudioProcessor& p);

    void paint (Graphics& g) override;

    void mouseMove (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    void mouseExit (const MouseEvent& e) override;

    void updateLabel();
    void createNewFilter();

private:
    void drawFreqLine (Graphics& g, float y, float dim = 6.0f);
    void drawWidthLine (Graphics& g, float top, float center, float x);

    void drawCursor (Graphics& g);
    void drawFilters (Graphics& g);

    float getTauForX (float x);
    float getXForTau (float tau);

    float getWidthForY (float y, float center);
    float getYForWidth (float bandwidth, float centerFreq);

    DrumFixerAudioProcessor& proc;

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
