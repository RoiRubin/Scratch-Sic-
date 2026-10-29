#pragma once
#include "PositionLaneView.h"
#include "GateLaneView.h"

// The scratching editor: a time ruler, the record position graph, and the fader graph under it,
// all sharing one time axis (the sample window), plus a moving playhead while the preview plays.
//
// Zoom with the mouse wheel (centred on the pointer), scroll with Shift + wheel, the scroll bar,
// or by dragging the ruler. Double-click the ruler, or use Fit, to see everything again.
class GraphEditor : public juce::Component, private juce::ScrollBar::Listener
{
public:
    explicit GraphEditor(ScratchSession& session);

    void setSnapStep(double step);
    void setPlayhead(double fraction);   // 0..1 across the graph; negative hides it
    void setTiming(double durationSeconds, double secondsPerBeat);   // for the ruler: seconds, and FL's beats
    void refresh();                      // the pattern was replaced or changed
    void resetZoom();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

private:
    class Playhead : public juce::Component
    {
    public:
        explicit Playhead(const TimeView& visible) : view(visible) { setInterceptsMouseClicks(false, false); }
        void setFraction(double newFraction);
        void paint(juce::Graphics& g) override;

    private:
        const TimeView& view;
        double fraction = -1.0;
    };

    void scrollBarMoved(juce::ScrollBar*, double newRangeStart) override;
    void viewChanged();

    ScratchSession& session;
    TimeView view;   // before the parts that read it
    PositionLaneView positionView;
    GateLaneView gateView;
    Playhead playhead;
    juce::ScrollBar scrollBar { false };

    double duration = 0.0, beatSeconds = 0.0;
    double dragStartViewStart = 0.0;
    bool draggingRuler = false;
};
