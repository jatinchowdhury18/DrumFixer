#include "FilterList.h"

FilterListComp::FilterListComp (DecayFilter& filt, FilterList* parentList) :
    filt (filt),
    parentList (parentList)
{
    auto setupLabel = [=] (Label& label, float& value)
    {
        addAndMakeVisible (label);
        label.setEditable (true);
        label.onTextChange = [=, &value, &label]
        { 
            value = label.getText().getFloatValue();
            // @TODO: tell filter to update from here...
            parentList->sendChangeMessage();
            updateLabels();
        };
    };

    auto& params = filt.getParams();
    setupLabel (freqLabel, params.centerFreq);
    setupLabel (bwLabel, params.bandwidth);
    setupLabel (tauLabel, params.desiredT60);

    updateLabels();

    setInterceptsMouseClicks (false, true);
}

void FilterListComp::updateLabels()
{
    const auto& params = filt.getParams();
    freqLabel.setText (String (params.centerFreq) + " Hz", dontSendNotification);
    bwLabel.setText (String (params.bandwidth) + " Hz", dontSendNotification);
    tauLabel.setText (String (params.desiredT60) + " sec", dontSendNotification);
}

void FilterListComp::resized()
{
    freqLabel.setBounds (0, 0, getWidth() / 4, getHeight());
    bwLabel.setBounds (freqLabel.getRight(), 0, getWidth() / 4, getHeight());
    tauLabel.setBounds (bwLabel.getRight(), 0, getWidth() / 4, getHeight());
}

//==================================================
FilterList::FilterList (DrumFixerAudioProcessor& p) :
    ListBox (String(), this),
    proc (p)
{
    setRowSelectedOnMouseDown (true);
}

void FilterList::paintListBoxItem (int row, Graphics& g, int width, int height, bool isSelected)
{
    if (isSelected)
        g.fillAll (Colours::darkred);
    else
        g.fillAll (Colours::black);
}

Component* FilterList::refreshComponentForRow (int rowNum, bool isSelected, Component* comp)
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
