#include "ExportTile.h"
#include "ScratchLookAndFeel.h"

namespace
{
    constexpr int dragThresholdPixels = 6;
}

void ExportTile::paint(juce::Graphics& g)
{
    const bool ready = session.hasRendered();
    const auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    g.setColour(ready ? Palette::accent : Palette::panel);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(ready ? Palette::accent.brighter(0.3f) : Palette::outline);
    g.drawRoundedRectangle(bounds, 8.0f, 1.5f);

    auto area = getLocalBounds().reduced(10, 8);
    g.setColour(juce::Colours::white.withAlpha(ready ? 1.0f : 0.45f));
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.drawText("DRAG INTO FL", area.removeFromTop(area.getHeight() * 2 / 5), juce::Justification::centred);

    g.setColour(juce::Colours::white.withAlpha(ready ? 0.9f : 0.45f));
    g.setFont(juce::FontOptions(12.0f));
    g.drawFittedText(ready ? "Click and drag this onto FL's playlist to drop your scratch in as an audio clip"
                           : "Load a sample first",
                     area, juce::Justification::centredTop, 3);
}

void ExportTile::mouseDrag(const juce::MouseEvent& e)
{
    if (dragging || e.getDistanceFromDragStart() < dragThresholdPixels || ! session.hasRendered())
        return;

    dragging = true;
    const auto file = session.exportCurrent();
    if (file.existsAsFile())
        juce::DragAndDropContainer::performExternalDragDropOfFiles({ file.getFullPathName() }, false, this);
    dragging = false;
}
