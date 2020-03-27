#ifndef FILTERLIST_H_INCLUDED
#define FILTERLIST_H_INCLUDED

#include "../PluginProcessor.h"

class FilterList : public ListBox,
                   public ListBoxModel,
                   public ChangeBroadcaster
{
public:
    FilterList (DrumFixerAudioProcessor& p);

    int getNumRows() override { return proc.getDecayFilters().size(); }
    void paintListBoxItem (int row, Graphics& g, int width, int height, bool isSelected) override;
    Component* refreshComponentForRow (int rowNum, bool isSelected, Component* comp) override;
    void deleteKeyPressed (int lastSelectedRow) override;

private:
    DrumFixerAudioProcessor& proc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterList)
};

class FilterListComp : public Component
{
public:
    FilterListComp (DecayFilter& filt, FilterList* parentList);

    void updateLabels();
    void resized() override;

    DecayFilter& getFilt() { return filt; }

private:
    DecayFilter& filt;
    FilterList* parentList;

    Label freqLabel;
    Label bwLabel;
    Label tauLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterListComp)
};

#endif // FILTERLIST_H_INCLUDED
