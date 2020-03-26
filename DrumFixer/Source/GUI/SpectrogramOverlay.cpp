#include "SpectrogramOverlay.h"
#include "../DSP/FFTUtils.h"

SpectrogramOverlay::SpectrogramOverlay (DrumFixerAudioProcessor& p) :
    proc (p)
{
    addChildComponent (cursorLabel);
    cursorLabel.setFont (12.0f);
    cursorLabel.setJustificationType (Justification::right);
}

void SpectrogramOverlay::drawFreqLine (Graphics& g, float y, float dim)
{
    const auto w = (float) getWidth();
    g.drawLine (0.0f, y, w, y, 2.0f);

    Path lTri;
    lTri.addTriangle (0.0f, y + dim, 0.0f, y - dim, dim, y);
    g.fillPath (lTri);

    Path rTri;
    rTri.addTriangle (w, y + dim, w, y - dim, w - dim, y);
    g.fillPath (rTri);
}

void SpectrogramOverlay::drawWidthLine (Graphics& g, float top, float center, float x)
{
    const auto height = 2 * (center - top);
    g.drawLine (x, top, x, top + height, 2.0f);

    const float dim = 5.0f;
    g.drawLine (x - dim, top, x + dim, top, 1.0f);
    g.drawLine (x - dim, top + height, x + dim, top + height, 1.0f);
}

void SpectrogramOverlay::drawCursor (Graphics& g)
{
    g.setColour (Colours::lightblue);
    if (state == ChooseFreq)
    {
        if (mouseY > 0.0f)
            drawFreqLine (g, mouseY);
    }
    else if (state == ChooseWidth)
    {
        drawFreqLine (g, freqY);

        if (mouseY < freqY && mouseY > 0.0f)
            drawWidthLine (g, mouseY, freqY, (float) getWidth() / 2);
    }
    else if (state == ChooseTau)
    {
        drawFreqLine (g, freqY);

        if (mouseX > 0.0f)
            drawWidthLine (g, widthY, freqY, mouseX);
        else
            drawWidthLine (g, widthY, freqY, (float) getWidth() / 2);
    }
}

void SpectrogramOverlay::drawFilters (Graphics& g)
{
    g.setColour (Colours::lightgrey);

    auto& filters = proc.getDecayFilters();
    for (auto& filt : filters)
    {
        auto& params = filt->getParams();
        auto filtY = (float) FFTUtils::freqToY (params.centerFreq, (float) getHeight());
        auto tauX = getXForTau (params.desiredT60);

        drawFreqLine (g, filtY);
        drawWidthLine (g, getYForWidth (params.bandwidth, params.centerFreq), filtY, tauX);
    }
}

void SpectrogramOverlay::paint (Graphics& g)
{
    drawFilters (g);
    drawCursor (g);

    g.setColour (Colours::white);
    g.drawRect (getLocalBounds().toFloat(), 2.0f);
}

void SpectrogramOverlay::mouseMove (const MouseEvent& e)
{
    cursorLabel.setVisible (true);
    mouseX = (float) e.x;
    mouseY = (float) e.y;
    updateLabel();
    repaint();
}

void SpectrogramOverlay::mouseExit (const MouseEvent& /*e*/)
{
    mouseX = -1.0f;
    mouseY = -1.0f;
    cursorLabel.setVisible (false);
    repaint();
}

void SpectrogramOverlay::mouseUp (const MouseEvent& e)
{
    switch (state)
    {
    case ChooseFreq:
        freqY = (float) e.y;
        state = ChooseWidth;
        break;
    
    case ChooseWidth:
        widthY = (float) e.y;
        state = ChooseTau;
        break;

    case ChooseTau:
        createNewFilter();
        state = ChooseFreq;
        break;
    }
    
    updateLabel();
    repaint();
}

float SpectrogramOverlay::getTauForX (float x)
{
    return 2.0f * pow (x / (float) getWidth(), 2.0f);
}

float SpectrogramOverlay::getXForTau (float tau)
{
    return pow (tau / 2.0f, 1.0f / 2.0f) * getWidth();
}

float SpectrogramOverlay::getWidthForY (float y, float center)
{
    return 2.0f * jmax (FFTUtils::yToFreq (y, (float) getHeight())
        - FFTUtils::yToFreq (center, (float) getHeight()), 0.0f);
}

float SpectrogramOverlay::getYForWidth (float bandwidth, float centerFreq)
{
    return (float) FFTUtils::freqToY((bandwidth / 2.0f) + centerFreq, (float) getHeight());
}

void SpectrogramOverlay::updateLabel()
{
    if (state == ChooseFreq)
    {
        auto freqHz = FFTUtils::yToFreq (mouseY, (float) getHeight());
        cursorLabel.setText (String (freqHz, 2) + " Hz", dontSendNotification);
        
        const int width = 100;
        const int height = 30;
        cursorLabel.setBounds (getWidth() - width, (int) mouseY - height, width, height);
    }

    if (state == ChooseWidth)
    {
        auto widthHz = getWidthForY (mouseY, freqY);
        cursorLabel.setText (String (widthHz, 2) + " Hz", dontSendNotification);

        const int width = 100;
        const int height = 30;
        cursorLabel.setBounds (getWidth() - width, (int) freqY - height, width, height);
    }

    if (state == ChooseTau)
    {
        auto tauSeconds = getTauForX (mouseX);

        cursorLabel.setText (String (tauSeconds, 2) + " sec", dontSendNotification);

        const int width = 100;
        const int height = 30;
        cursorLabel.setBounds (getWidth() - width, (int) freqY - height, width, height);
    }
}

void SpectrogramOverlay::createNewFilter()
{
    auto freqHz = FFTUtils::yToFreq (freqY, (float) getHeight());
    auto widthHz = getWidthForY (mouseY, freqY);
    auto tauSeconds = getTauForX (mouseX);

    DecayFilter::Params params (freqHz, widthHz, tauSeconds);
    proc.addDecayFilter (params);
}
