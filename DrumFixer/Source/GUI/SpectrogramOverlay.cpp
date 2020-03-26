#include "SpectrogramOverlay.h"
#include "../DSP/FFTUtils.h"

SpectrogramOverlay::SpectrogramOverlay()
{
    addChildComponent (cursorLabel);
    cursorLabel.setFont (12.0f);
    cursorLabel.setJustificationType (Justification::right);
}

void SpectrogramOverlay::paint (Graphics& g)
{
    auto drawLine = [=, &g] (float y, float dim = 8.0f)
    {
        const auto w = (float) getWidth();
        g.drawLine (0.0f, y, w, y, 2.0f);

        Path lTri;
        lTri.addTriangle (0.0f, y + dim, 0.0f, y - dim, dim, y);
        g.fillPath (lTri);

        Path rTri;
        rTri.addTriangle (w, y + dim, w, y - dim, w - dim, y);
        g.fillPath (rTri);
    };

    auto drawWidthLine = [=, &g] (float top, float center, float x)
    {
        const auto height = 2 * (center - top);
        g.drawLine (x, top, x, top + height, 2.0f);

        const float dim = 5.0f;
        g.drawLine (x - dim, top, x + dim, top, 1.0f);
        g.drawLine (x - dim, top + height, x + dim, top + height, 1.0f);
    };

    g.setColour (Colours::lightblue);

    if (state == ChooseFreq)
    {
        if (mouseY > 0.0f)
            drawLine (mouseY);
    }
    else if (state == ChooseWidth)
    {
        drawLine (freqY);

        if (mouseY < freqY && mouseY > 0.0f)
            drawWidthLine (mouseY, freqY, (float) getWidth() / 2);
    }
    else if (state == ChooseTau)
    {
        drawLine (freqY);

        if (mouseX > 0.0f)
            drawWidthLine (widthY, freqY, mouseX);
        else
            drawWidthLine (widthY, freqY, (float) getWidth() / 2);
    }

    g.setColour (Colours::white);
    g.drawRect (getLocalBounds().toFloat(), 2.0f);
}

void SpectrogramOverlay::mouseMove (const MouseEvent& e)
{
    cursorLabel.setVisible (true);
    mouseX = (float) e.x;
    mouseY = (float) e.y;
    resetLabel();
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
        state = ChooseFreq;
        break;
    }
    
    resetLabel();
    repaint();
}

void SpectrogramOverlay::resetLabel()
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
        auto widthHz = jmax (FFTUtils::yToFreq (mouseY, (float) getHeight())
            - FFTUtils::yToFreq (freqY, (float) getHeight()), 0.0f);

        cursorLabel.setText (String (widthHz, 2) + " Hz", dontSendNotification);

        const int width = 100;
        const int height = 30;
        cursorLabel.setBounds (getWidth() - width, (int) freqY - height, width, height);
    }

    if (state == ChooseTau)
    {
        auto tauSeconds = 2.0f * pow (mouseX / (float) getWidth(), 3.0f);

        cursorLabel.setText (String (tauSeconds, 2) + " sec", dontSendNotification);

        const int width = 100;
        const int height = 30;
        cursorLabel.setBounds (getWidth() - width, (int) freqY - height, width, height);
    }
}
