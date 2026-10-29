#pragma once
#include "scratch/ScratchSession.h"
#include "GridMapper.h"

// The fader graph. Drag across empty space to open the fader there, right-drag to cut a gap,
// drag a block's edges to resize it or its middle to move it, double-click a block to remove it.
class GateLaneView : public juce::Component
{
public:
    GateLaneView(ScratchSession& sessionToEdit, const TimeView& visible) : session(sessionToEdit), view(visible) {}

    void setSnapStep(double step) { snapStep = step; repaint(); }

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

private:
    enum class Mode { none, paint, erase, resizeStart, resizeEnd, move };

    GridMapper mapper() const;
    double minimumLength() const noexcept;
    int blockAt(const GateLane& lane, float x, bool& onStartEdge, bool& onEndEdge) const;
    void edited();

    ScratchSession& session;
    const TimeView& view;
    double snapStep = 1.0 / 16.0;

    // Every drag restarts from the lane as it was on mouse-down, so edits never pile up
    Mode mode = Mode::none;
    GateLane base;
    double anchor = 0.0, grabOffset = 0.0;
    int blockIndex = -1;
    bool dragged = false;
};
