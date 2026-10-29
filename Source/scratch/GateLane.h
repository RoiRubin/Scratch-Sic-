#pragma once
#include <juce_data_structures/juce_data_structures.h>
#include <vector>

// A stretch of time where the fader is open (sound on). Times run 0..1 across the whole graph.
struct GateBlock
{
    double start = 0.0;
    double end = 0.0;
};

// The bottom graph: when the crossfader lets sound through. Anything outside a block is cut.
class GateLane
{
public:
    GateLane() = default;
    explicit GateLane(std::vector<GateBlock> newBlocks);

    static GateLane open();   // fader open the whole time

    const std::vector<GateBlock>& getBlocks() const noexcept { return blocks; }
    bool isOpenAt(double time) const noexcept;

    void paint(double from, double to);   // open the fader over this range
    void erase(double from, double to);   // cut the sound over this range
    void setBlock(int index, double start, double end);   // call normalise() once finished editing
    void removeBlock(int index);
    void scaleTime(double factor);
    void normalise();   // sort, merge overlaps, drop empty blocks

    juce::ValueTree toValueTree() const;
    static GateLane fromValueTree(const juce::ValueTree& tree);

private:
    std::vector<GateBlock> blocks;
};
