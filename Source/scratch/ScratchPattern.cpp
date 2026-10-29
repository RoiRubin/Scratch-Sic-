#include "ScratchPattern.h"

ScratchPattern::ScratchPattern()
    : name("New scratch"), category("Custom"), gate(GateLane::open())
{
}

ScratchPattern ScratchPattern::create(const juce::String& name, const juce::String& category, PositionLane position, GateLane gate)
{
    ScratchPattern p;
    p.name = name;
    p.category = category;
    p.position = std::move(position);
    p.gate = std::move(gate);
    return p;
}

juce::ValueTree ScratchPattern::toValueTree() const
{
    juce::ValueTree tree("Scratch");
    tree.setProperty("name", name, nullptr);
    tree.setProperty("category", category, nullptr);
    tree.appendChild(position.toValueTree(), nullptr);
    tree.appendChild(gate.toValueTree(), nullptr);
    return tree;
}

juce::String ScratchPattern::toXmlString() const
{
    return toValueTree().toXmlString();
}

std::optional<ScratchPattern> ScratchPattern::fromValueTree(const juce::ValueTree& tree)
{
    if (! tree.hasType("Scratch"))
        return std::nullopt;

    ScratchPattern p;
    p.name = tree.getProperty("name", "Untitled").toString();
    p.category = tree.getProperty("category", "Custom").toString();
    p.position = PositionLane::fromValueTree(tree.getChildWithName("Position"));
    p.gate = GateLane::fromValueTree(tree.getChildWithName("Gate"));

    // version 0.2 scratches were drawn in beats with their own length; stretch them to fill the graph
    if (tree.hasProperty("length"))
    {
        const double factor = 1.0 / juce::jmax(0.01, (double) tree.getProperty("length"));
        p.position.scaleTime(factor);
        p.gate.scaleTime(factor);
    }
    return p;
}

std::optional<ScratchPattern> ScratchPattern::fromXmlString(const juce::String& xml)
{
    return fromValueTree(juce::ValueTree::fromXml(xml));
}
