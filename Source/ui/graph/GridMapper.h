#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/ScratchLookAndFeel.h"

// The part of the graph that is on screen. Time runs 0..1 across the whole graph (the sample window);
// zooming in shows a smaller span of it.
struct TimeView
{
    static constexpr double minSpan = 0.01;

    double start = 0.0, span = 1.0;

    double end() const noexcept { return start + span; }
    void clamp() noexcept
    {
        span = juce::jlimit(minSpan, 1.0, span);
        start = juce::jlimit(0.0, 1.0 - span, start);
    }
};

// Converts between time / values and pixels for one graph lane, so every lane lines up
struct GridMapper
{
    static constexpr float sidePadding = 12.0f;   // keeps points on the very first and last moment fully visible

    juce::Rectangle<float> area;
    TimeView view;

    GridMapper(juce::Rectangle<int> laneBounds, float verticalPadding, const TimeView& visible)
        : area(laneBounds.toFloat().reduced(sidePadding, verticalPadding)), view(visible) {}

    float timeToX(double time) const noexcept
    {
        return area.getX() + (float) ((time - view.start) / view.span) * area.getWidth();
    }
    double xToTime(float x) const noexcept
    {
        return juce::jlimit(0.0, 1.0, view.start + (double) ((x - area.getX()) / area.getWidth()) * view.span);
    }
    float valueToY(float value) const noexcept { return area.getBottom() - value * area.getHeight(); }
    float yToValue(float y) const noexcept { return juce::jlimit(0.0f, 1.0f, (area.getBottom() - y) / area.getHeight()); }

    double snap(double time, double step) const noexcept
    {
        return step > 0.0 ? juce::jlimit(0.0, 1.0, std::round(time / step) * step) : time;
    }
};

// Spacing of the strong grid lines and ruler labels: a power of two, giving about 4-8 across the screen
inline double coarseGridStep(const TimeView& view)
{
    return std::pow(2.0, std::floor(std::log2(view.span / 4.0)));
}

// Vertical lines: strong on the coarse steps, faint on the snap divisions
inline void drawTimeGrid(juce::Graphics& g, const GridMapper& map, double snapStep)
{
    const auto drawLines = [&](double step)
    {
        for (double t = std::ceil(map.view.start / step) * step; t <= map.view.end() + 1.0e-9; t += step)
            g.drawVerticalLine((int) map.timeToX(t), map.area.getY(), map.area.getBottom());
    };

    const double snap = snapStep > 0.0 ? snapStep : 1.0 / 16.0;
    if (map.area.getWidth() * (float) (snap / map.view.span) >= 6.0f)
    {
        g.setColour(Palette::outline.withAlpha(0.5f));
        drawLines(snap);
    }

    g.setColour(Palette::dimText.withAlpha(0.55f));
    drawLines(coarseGridStep(map.view));
}
