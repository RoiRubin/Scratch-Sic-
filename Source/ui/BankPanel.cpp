#include "BankPanel.h"
#include "ScratchLookAndFeel.h"

namespace
{
    const juce::StringArray categories { "Baby", "Chirp", "Transformer", "Flare", "Scribble", "Custom" };
    constexpr int rowHeight = 30;
}

BankPanel::BankPanel(ScratchSession& s)
    : session(s)
{
    list.setRowHeight(rowHeight);
    list.setColour(juce::ListBox::backgroundColourId, Palette::panel);
    list.setColour(juce::ListBox::outlineColourId, Palette::outline);
    list.setOutlineThickness(1);
    addAndMakeVisible(list);

    nameEditor.setTextToShowWhenEmpty("Name your scratch", Palette::dimText);
    nameEditor.setColour(juce::TextEditor::backgroundColourId, Palette::panel);
    nameEditor.setColour(juce::TextEditor::textColourId, Palette::text);
    nameEditor.setColour(juce::TextEditor::outlineColourId, Palette::outline);
    addAndMakeVisible(nameEditor);

    categoryBox.addItemList(categories, 1);
    categoryBox.setSelectedId(categories.indexOf("Custom") + 1, juce::dontSendNotification);
    addAndMakeVisible(categoryBox);

    addButton.onClick = [this] { addCurrentToBank(); };
    deleteButton.onClick = [this] { deleteSelected(); };
    addAndMakeVisible(addButton);
    addAndMakeVisible(deleteButton);
}

void BankPanel::paint(juce::Graphics& g)
{
    g.setColour(Palette::text);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText("SCRATCH BANK", getLocalBounds().removeFromTop(22), juce::Justification::centredLeft);
}

void BankPanel::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(24);

    auto bottom = area.removeFromBottom(98);
    list.setBounds(area);

    bottom.removeFromTop(8);
    nameEditor.setBounds(bottom.removeFromTop(26));
    bottom.removeFromTop(6);
    categoryBox.setBounds(bottom.removeFromTop(26));
    bottom.removeFromTop(6);
    auto buttons = bottom.removeFromTop(26);
    addButton.setBounds(buttons.removeFromLeft(buttons.getWidth() * 2 / 3 - 3));
    buttons.removeFromLeft(6);
    deleteButton.setBounds(buttons);
}

int BankPanel::getNumRows()
{
    return (int) session.getBank().getEntries().size();
}

void BankPanel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected)
{
    const auto& entries = session.getBank().getEntries();
    if (row < 0 || row >= (int) entries.size())
        return;

    const auto& entry = entries[(size_t) row];
    if (selected)
    {
        g.setColour(Palette::accent.withAlpha(0.25f));
        g.fillRect(0, 0, width, height);
    }

    g.setColour(Palette::text);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText(entry.pattern.getName(), 10, 0, width - 90, height, juce::Justification::centredLeft);

    g.setColour(Palette::dimText);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText(entry.pattern.getCategory(), width - 90, 0, 80, height, juce::Justification::centredRight);

    if (! entry.factory)
    {
        g.setColour(Palette::accent);
        g.fillEllipse(3.0f, height * 0.5f - 2.0f, 4.0f, 4.0f);   // marks the user's own scratches
    }
}

void BankPanel::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    const auto& entries = session.getBank().getEntries();
    if (row >= 0 && row < (int) entries.size())
    {
        session.loadPattern(entries[(size_t) row].pattern);
        nameEditor.setText(entries[(size_t) row].pattern.getName(), juce::dontSendNotification);
        const int category = categories.indexOf(entries[(size_t) row].pattern.getCategory());
        categoryBox.setSelectedId(category >= 0 ? category + 1 : categories.indexOf("Custom") + 1, juce::dontSendNotification);
    }
}

juce::var BankPanel::getDragSourceDescription(const juce::SparseSet<int>& selectedRows)
{
    return selectedRows.isEmpty() ? juce::var() : juce::var("pattern:" + juce::String(selectedRows[0]));
}

void BankPanel::deleteKeyPressed(int)
{
    deleteSelected();
}

void BankPanel::addCurrentToBank()
{
    auto pattern = session.getPattern();
    const auto name = nameEditor.getText().trim();
    if (name.isEmpty())
    {
        nameEditor.grabKeyboardFocus();
        return;
    }

    pattern.setName(name);
    pattern.setCategory(categoryBox.getText());
    if (session.getBank().addUserPattern(pattern))
    {
        session.getPattern().setName(name);
        session.getPattern().setCategory(pattern.getCategory());
        session.patternEdited();
        session.sendChangeMessage();
        list.updateContent();
        list.selectRow((int) session.getBank().getEntries().size() - 1);
    }
}

void BankPanel::deleteSelected()
{
    const int row = list.getSelectedRow();
    if (session.getBank().removeUserPattern(row))
    {
        list.updateContent();
        list.deselectAllRows();
    }
}
