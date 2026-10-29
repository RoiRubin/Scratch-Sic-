#pragma once
#include <juce_data_structures/juce_data_structures.h>
#include <vector>

// One corner of the record position line.
// time runs 0..1 across the whole graph (the sample window); value is 0..1 across the window too.
// curve (-1..1) bends, and smooth chooses the character of, the stretch of line from this point to the next one.
struct PositionPoint
{
    double time = 0.0;
    float value = 0.5f;
    float curve = 0.0f;
    bool smooth = true;   // true: flows like a hand moving the record. false: runs in a straight line.
};

// The top graph: where the needle is on the record at every moment. Always has at least two points.
// Smooth stretches of line flow through their points the way a hand would move the record: speeding up
// and slowing down inside each stroke and turning round gently at the peaks. Each stretch is smooth or
// straight independently.
class PositionLane
{
public:
    PositionLane();   // a straight line across the window: the sample at normal speed
    explicit PositionLane(std::vector<PositionPoint> newPoints);

    const std::vector<PositionPoint>& getPoints() const noexcept { return points; }
    bool areAllSmooth() const noexcept;
    void setAllSmooth(bool shouldSmooth);
    void setSegmentSmooth(int index, bool shouldSmooth);   // the line from point index to the next

    float valueAt(double time) const noexcept;
    float valueInSegment(int index, double t) const noexcept;   // t = 0..1 along the line from point index to the next

    int addPoint(double time, float value);   // returns the new point's index
    void removePoint(int index);
    void removePoints(const std::vector<int>& indices);   // always leaves at least two points

    // The two calls below take the points as they were when the drag started (original) and the indices
    // of the selected points in ascending order. Points never pass the unselected points next to them.

    // Moves the selected points together by the same amount. The shape is kept: if any point would run out
    // of room, the whole move is shortened instead of squashing the shape.
    void moveSelection(const std::vector<PositionPoint>& original, const std::vector<int>& selected,
                       double deltaTime, float deltaValue);

    // Drags one outer end of the selection to a new spot while the other end stays put. The points between
    // follow in proportion, so a flat run tilts into a diagonal and a run can be stretched or squeezed.
    void stretchSelection(const std::vector<PositionPoint>& original, const std::vector<int>& selected,
                          bool draggingLastEnd, double newTime, float newValue);

    void setCurve(int index, float curve);
    void scaleTime(double factor);

    juce::ValueTree toValueTree() const;
    static PositionLane fromValueTree(const juce::ValueTree& tree);

private:
    void updateSlopes();

    std::vector<PositionPoint> points;
    std::vector<double> slopes;   // how steeply the smoothed line passes through each point
};
