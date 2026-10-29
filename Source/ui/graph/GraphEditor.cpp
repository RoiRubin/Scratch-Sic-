#include "GraphEditor.h"

namespace
{
    constexpr int rulerHeight = 18, laneGap = 6, gateHeight = 74, scrollBarHeight = 12;
    constexpr int maxBeatTicks = 512;
    constexpr double wheelZoomSpeed = 1.2, wheelPanSpeed = 0.5;
}

GraphEditor::GraphEditor(ScratchSession& s)
    : session(s), positionView(s, view), gateView(s, view), playhead(view)
{
    addAndMakeVisible(positionView);
    addAndMakeVisible(gateView);
    addAndMakeVisible(playhead);

    scrollBar.addListener(this);
    scrollBar.setAutoHide(false);
    addAndMakeVisible(scrollBar);
    viewChanged();
}

void GraphEditor::setSnapStep(double step)
{
    positionView.setSnapStep(step);
    gateView.setSnapStep(step);
}

void GraphEditor::setPlayhead(double fraction)
{
    playhead.setFraction(fraction);
}

void GraphEditor::setTiming(double durationSeconds, double secondsPerBeat)
{
    if (juce::approximatelyEqual(durationSeconds, duration) && juce::approximatelyEqual(secondsPerBeat, beatSeconds))
        return;
    duration = durationSeconds;
    beatSeconds = secondsPerBeat;
    repaint(getLocalBounds().withHeight(rulerHeight));
}

void GraphEditor::refresh()
{
    positionView.clearSelection();   // the pattern was replaced, so the old selection no longer means anything
    repaint();
    positionView.repaint();
    gateView.repaint();
}

void GraphEditor::resetZoom()
{
    view = {};
    viewChanged();
}

void GraphEditor::viewChanged()
{
    view.clamp();
    scrollBar.setRangeLimits(0.0, 1.0);
    scrollBar.setCurrentRange(view.start, view.span, juce::dontSendNotification);
    scrollBar.setEnabled(view.span < 1.0);
    repaint();
    positionView.repaint();
    gateView.repaint();
    playhead.repaint();
}

void GraphEditor::scrollBarMoved(juce::ScrollBar*, double newRangeStart)
{
    view.start = newRangeStart;
    view.clamp();
    repaint();
    positionView.repaint();
    gateView.repaint();
    playhead.repaint();
}

void GraphEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(rulerHeight);
    scrollBar.setBounds(area.removeFromBottom(scrollBarHeight));
    area.removeFromBottom(2);
    playhead.setBounds(area);
    gateView.setBounds(area.removeFromBottom(gateHeight));
    area.removeFromBottom(laneGap);
    positionView.setBounds(area);
}

void GraphEditor::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    const bool panning = e.mods.isShiftDown() || std::abs(wheel.deltaX) > std::abs(wheel.deltaY);
    if (panning)
    {
        const double amount = std::abs(wheel.deltaX) > std::abs(wheel.deltaY) ? wheel.deltaX : wheel.deltaY;
        view.start -= amount * wheelPanSpeed * view.span;
    }
    else
    {
        // keep the moment under the pointer where it is while the span shrinks or grows
        const GridMapper map(getLocalBounds(), 0.0f, view);
        const double pivot = map.xToTime((float) e.position.x);
        const double pivotFraction = (pivot - view.start) / view.span;
        view.span *= std::exp(-wheel.deltaY * wheelZoomSpeed);
        view.span = juce::jlimit(TimeView::minSpan, 1.0, view.span);
        view.start = pivot - pivotFraction * view.span;
    }
    viewChanged();
}

void GraphEditor::mouseDown(const juce::MouseEvent& e)
{
    draggingRuler = e.y < rulerHeight;
    dragStartViewStart = view.start;
}

void GraphEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (! draggingRuler)
        return;

    const GridMapper map(getLocalBounds(), 0.0f, view);
    view.start = dragStartViewStart - e.getDistanceFromDragStartX() / map.area.getWidth() * view.span;
    view.clamp();
    scrollBar.setCurrentRange(view.start, view.span, juce::dontSendNotification);
    repaint();
    positionView.repaint();
    gateView.repaint();
    playhead.repaint();
}

void GraphEditor::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (e.y < rulerHeight)
        resetZoom();
}

void GraphEditor::paint(juce::Graphics& g)
{
    if (duration <= 0.0)
        return;

    const GridMapper map(getLocalBounds().withHeight(rulerHeight), 0.0f, view);

    // little ticks where FL's beats fall, so the scratch can be lined up with the song
    if (beatSeconds > 0.0)
    {
        const double beatStep = beatSeconds / duration;   // as a fraction of the graph
        g.setColour(Palette::accent.withAlpha(0.85f));
        int ticks = 0;
        for (double t = std::ceil(view.start / beatStep) * beatStep; t <= view.end() + 1.0e-9 && ticks < maxBeatTicks; t += beatStep, ++ticks)
            g.fillRect(map.timeToX(t) - 0.5f, (float) rulerHeight - 6.0f, 1.5f, 6.0f);
    }

    const double step = coarseGridStep(view);
    const int decimals = duration * step < 0.01 ? 3 : 2;
    g.setColour(Palette::dimText);
    g.setFont(juce::FontOptions(11.0f));
    for (double t = std::ceil(view.start / step) * step; t <= view.end() + 1.0e-9; t += step)
    {
        const int x = (int) map.timeToX(t);
        g.drawText(juce::String(duration * t, decimals) + " s", x - 30, 0, 60, rulerHeight - 4, juce::Justification::centred);
    }
}

void GraphEditor::Playhead::setFraction(double newFraction)
{
    if (juce::approximatelyEqual(newFraction, fraction))
        return;
    fraction = newFraction;
    repaint();
}

void GraphEditor::Playhead::paint(juce::Graphics& g)
{
    if (fraction < 0.0 || fraction < view.start || fraction > view.end())
        return;

    const GridMapper map(getLocalBounds(), 0.0f, view);
    g.setColour(Palette::text.withAlpha(0.85f));
    g.fillRect(map.timeToX(fraction) - 0.5f, 0.0f, 1.5f, (float) getHeight());
}
