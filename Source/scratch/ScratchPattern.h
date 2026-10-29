#pragma once
#include "PositionLane.h"
#include "GateLane.h"
#include <optional>

// One complete scratch: the record position line and the fader blocks. It always spans the whole graph,
// which is the sample window, so how long it lasts is decided by the Window Length knob.
// This is what the editor draws, what the bank stores, and what gets rendered to sound.
class ScratchPattern
{
public:
    ScratchPattern();

    const juce::String& getName() const noexcept { return name; }
    const juce::String& getCategory() const noexcept { return category; }
    PositionLane& getPosition() noexcept { return position; }
    const PositionLane& getPosition() const noexcept { return position; }
    GateLane& getGate() noexcept { return gate; }
    const GateLane& getGate() const noexcept { return gate; }

    void setName(const juce::String& newName) { name = newName; }
    void setCategory(const juce::String& newCategory) { category = newCategory; }

    static ScratchPattern create(const juce::String& name, const juce::String& category, PositionLane position, GateLane gate);

    juce::ValueTree toValueTree() const;
    juce::String toXmlString() const;
    static std::optional<ScratchPattern> fromValueTree(const juce::ValueTree& tree);
    static std::optional<ScratchPattern> fromXmlString(const juce::String& xml);

private:
    juce::String name, category;
    PositionLane position;
    GateLane gate;
};
