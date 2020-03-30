#include "FilterList.h"

FilterListHeader::FilterListHeader()
{
    addAndMakeVisible (freqLabel);
    freqLabel.setText ("Center Freq.", dontSendNotification);

    addAndMakeVisible (bwLabel);
    bwLabel.setText ("Bandwidth", dontSendNotification);

    addAndMakeVisible (tauLabel);
    tauLabel.setText ("Desired T60", dontSendNotification);

    setSize (100, 30);
}

void FilterListHeader::resized()
{
    freqLabel.setBounds (0,                    0, 3 * getWidth() / 10, getHeight());
    bwLabel.setBounds   (freqLabel.getRight(), 0, 3 * getWidth() / 10, getHeight());
    tauLabel.setBounds  (bwLabel.getRight(),   0, 3 * getWidth() / 10, getHeight());
}

void FilterListHeader::paint (Graphics& g)
{
    g.fillAll (Colours::black);

    g.setColour (Colours::white);
    const auto y = (float) getHeight() - 2.0f;
    g.drawLine (0.0f, y, (float) getWidth(), y);
}

//==================================================
FilterListComp::FilterListComp (DecayFilter& filt, FilterList* parentList) :
    filt (filt)
{
    auto setupLabel = [=, &filt] (Label& label, float& value)
    {
        addAndMakeVisible (label);
        label.setEditable (true);
        label.onTextChange = [=, &value, &label, &filt]
        { 
            value = label.getText().getFloatValue();
            filt.updateFilter();
            parentList->sendChangeMessage();
            updateLabels();
        };
    };

    auto& params = filt.getParams();
    setupLabel (freqLabel, params.centerFreq);
    setupLabel (bwLabel,   params.bandwidth);
    setupLabel (tauLabel,  params.desiredT60);

    updateLabels();

    setInterceptsMouseClicks (false, true);
}

void FilterListComp::updateLabels()
{
    const auto& params = filt.getParams();
    freqLabel.setText (String (params.centerFreq, 2) + " Hz", dontSendNotification);
    bwLabel.setText   (String (params.bandwidth , 2) + " Hz", dontSendNotification);
    tauLabel.setText  (String (params.desiredT60, 4) + " sec", dontSendNotification);
}

void FilterListComp::resized()
{
    freqLabel.setBounds (0,                    0, 3 * getWidth() / 10, getHeight());
    bwLabel.setBounds   (freqLabel.getRight(), 0, 3 * getWidth() / 10, getHeight());
    tauLabel.setBounds  (bwLabel.getRight(),   0, 3 * getWidth() / 10, getHeight());
}

//==================================================
FilterList::FilterList (DrumFixerAudioProcessor& p) :
    ListBox (String(), this),
    proc (p)
{
    setHeaderComponent (new FilterListHeader);
    setRowSelectedOnMouseDown (true);
    setColour (ListBox::backgroundColourId, Colours::black);
}

void FilterList::paintListBoxItem (int /*row*/, Graphics& g, int width, int /*height*/, bool isSelected)
{
    if (isSelected)
        g.fillAll (Colours::green);
    else
        g.fillAll (Colours::black);

    g.setColour (Colours::green.darker (isSelected ? 0.8f : 0.0f));
    g.fillRect ((float) width - 20.0f, 10.0f, 10.0f, 10.0f);
}

Component* FilterList::refreshComponentForRow (int rowNum, bool /*isSelected*/, Component* comp)
{
    if (rowNum >= proc.getDecayFilters().size())
    {
        delete comp;
        return nullptr;
    }

    auto compCast = dynamic_cast<FilterListComp*> (comp);

    if (compCast != nullptr) // try to use existing component...
    {
        if (proc.getDecayFilters()[rowNum] == &compCast->getFilt())
            return comp;

        delete compCast;
    }

    // create new component
    auto filt = proc.getDecayFilters()[rowNum];
    return new FilterListComp (*filt, this);
}

void FilterList::deleteKeyPressed (int lastSelectedRow)
{
    if (lastSelectedRow >= proc.getDecayFilters().size())
        return;

    proc.getDecayFilters().remove (lastSelectedRow, true);
    updateContent();
    sendChangeMessage();
}

void FilterList::selectedRowsChanged (int lastSelected)
{
    auto& filts = proc.getDecayFilters();
    for (int i = 0; i < filts.size(); ++i)
        filts[i]->setSelected (i == lastSelected);

    getParentComponent()->repaint();
}
