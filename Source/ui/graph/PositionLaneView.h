#pragma once
#include "scratch/ScratchSession.h"
#include "GridMapper.h"

// The record position graph.
//   Click empty space: add a point.   Drag a point: move it.   Drag the small circle on a line: bend it.
//   Double-click a point (or right-click it): remove it.
//   Right-click the circle on a line: choose whether that stretch of line is smooth or straight.
//   A smooth stretch has a round circle, a straight one a diamond.
//   Shift + click a point, or Shift + drag a box: select several points. Dragging any selected point moves them all.
//   Ctrl + drag an outer dot of a selection: tilt and stretch the whole selection, the other outer dot stays put.
class PositionLaneView : public juce::Component
{
public:
    PositionLaneView(ScratchSession& sessionToEdit, const TimeView& visible) : session(sessionToEdit), view(visible) {}

    void setSnapStep(double step) { snapStep = step; repaint(); }
    void clearSelection();

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

private:
    enum class Mode { none, move, stretch, curve, marquee };

    GridMapper mapper() const;
    int hitPoint(juce::Point<float> p) const;
    int hitHandle(juce::Point<float> p) const;
    juce::Point<float> handlePosition(const GridMapper& map, int segment) const;
    bool isSelected(int index) const;
    void toggleSelected(int index);
    void pruneSelection();
    void showSegmentMenu(int segment);
    void edited();

    ScratchSession& session;
    const TimeView& view;
    double snapStep = 1.0 / 16.0;

    std::vector<int> selected;   // point indices, ascending
    Mode mode = Mode::none;
    int dragIndex = -1, hoverPoint = -1, hoverHandle = -1;

    // moving / stretching: the points as they were at mouse-down, and which one was grabbed
    std::vector<PositionPoint> dragOriginal;
    juce::Point<float> grabOffset;   // where on the dot it was grabbed, so a drag doesn't make it jump
    bool stretchLastEnd = false;

    // bending a line
    float curveDragStartY = 0.0f, curveDragStartValue = 0.0f;

    // box selection
    juce::Point<float> marqueeStart;
    juce::Rectangle<float> marquee;
    std::vector<int> selectedBeforeMarquee;
};
