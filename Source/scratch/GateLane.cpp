#include "GateLane.h"
#include <algorithm>

namespace
{
    constexpr double tiny = 1.0e-6;
}

GateLane::GateLane(std::vector<GateBlock> newBlocks)
    : blocks(std::move(newBlocks))
{
    normalise();
}

GateLane GateLane::open()
{
    return GateLane({ { 0.0, 1.0 } });
}

bool GateLane::isOpenAt(double time) const noexcept
{
    for (const auto& b : blocks)
        if (time >= b.start && time < b.end)
            return true;
    return false;
}

void GateLane::paint(double from, double to)
{
    blocks.push_back({ juce::jmin(from, to), juce::jmax(from, to) });
    normalise();
}

void GateLane::erase(double from, double to)
{
    const double lo = juce::jmin(from, to), hi = juce::jmax(from, to);
    std::vector<GateBlock> kept;
    for (const auto& b : blocks)
    {
        if (b.end <= lo || b.start >= hi)
        {
            kept.push_back(b);
            continue;
        }
        if (b.start < lo) kept.push_back({ b.start, lo });
        if (b.end > hi) kept.push_back({ hi, b.end });
    }
    blocks = std::move(kept);
    normalise();
}

void GateLane::setBlock(int index, double start, double end)
{
    if (index >= 0 && index < (int) blocks.size())
        blocks[(size_t) index] = { juce::jmin(start, end), juce::jmax(start, end) };
}

void GateLane::removeBlock(int index)
{
    if (index >= 0 && index < (int) blocks.size())
        blocks.erase(blocks.begin() + index);
}

void GateLane::scaleTime(double factor)
{
    for (auto& b : blocks)
    {
        b.start = juce::jlimit(0.0, 1.0, b.start * factor);
        b.end = juce::jlimit(0.0, 1.0, b.end * factor);
    }
    normalise();
}

void GateLane::normalise()
{
    std::sort(blocks.begin(), blocks.end(), [](const auto& a, const auto& b) { return a.start < b.start; });

    std::vector<GateBlock> merged;
    for (const auto& b : blocks)
    {
        if (b.end - b.start < tiny)
            continue;
        if (! merged.empty() && b.start <= merged.back().end + tiny)
            merged.back().end = juce::jmax(merged.back().end, b.end);
        else
            merged.push_back(b);
    }
    blocks = std::move(merged);
}

juce::ValueTree GateLane::toValueTree() const
{
    juce::ValueTree tree("Gate");
    for (const auto& b : blocks)
    {
        juce::ValueTree node("Block");
        node.setProperty("start", b.start, nullptr);
        node.setProperty("end", b.end, nullptr);
        tree.appendChild(node, nullptr);
    }
    return tree;
}

GateLane GateLane::fromValueTree(const juce::ValueTree& tree)
{
    std::vector<GateBlock> loaded;
    for (const auto& node : tree)
        loaded.push_back({ (double) node.getProperty("start"), (double) node.getProperty("end") });
    return GateLane(std::move(loaded));
}
