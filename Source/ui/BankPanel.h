#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "scratch/ScratchSession.h"

// The scratch bank: click a scratch to load it into the editor, or drag it onto the waveform.
// Below the list, the scratch in the editor can be saved into the bank under a name and category.
class BankPanel : public juce::Component, private juce::ListBoxModel
{
public:
    explicit BankPanel(ScratchSession& session);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    juce::var getDragSourceDescription(const juce::SparseSet<int>& selectedRows) override;
    void deleteKeyPressed(int row) override;

    void addCurrentToBank();
    void deleteSelected();

    ScratchSession& session;
    juce::ListBox list { "bank", this };
    juce::TextEditor nameEditor;
    juce::ComboBox categoryBox;
    juce::TextButton addButton { "Add to bank" }, deleteButton { "Delete" };
};
