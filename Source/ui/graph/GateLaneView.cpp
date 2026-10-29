#include "GateLaneView.h"

namespace
{
    constexpr float verticalPadding = 10.0f;
    constexpr float edgeGrabPixels = 7.0f;
    constexpr double smallestBlock = 0.01;   // fraction of the graph
}

GridMapper GateLaneView::mapper() const
{
    return { getLocalBounds(), verticalPadding, view };
}

double GateLaneView::minimumLength() const noexcept
{
    return juce::jmax(snapStep, smallestBlock);
}

int GateLaneView::blockAt(const GateLane& lane, float x, bool& onStartEdge, bool& onEndEdge) const
{
    const auto map = mapper();
    onStartEdge = onEndEdge = false;
    const auto& blocks = lane.getBlocks();
    for (int i = 0; i < (int) blocks.size(); ++i)
    {
        const float left = map.timeToX(blocks[(size_t) i].start), right = map.timeToX(blocks[(size_t) i].end);
        if (std::abs(x - left) <= edgeGrabPixels) { onStartEdge = true; return i; }
        if (std::abs(x - right) <= edgeGrabPixels) { onEndEdge = true; return i; }
        if (x > left && x < right) return i;
    }
    return -1;
}

void GateLaneView::edited()
{
    session.patternEdited();
    repaint();
}

void GateLaneView::paint(juce::Graphics& g)
{
    g.setColour(Palette::panel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

    const auto map = mapper();
    drawTimeGrid(g, map, snapStep);

    for (const auto& b : session.getPattern().getGate().getBlocks())
    {
        const auto r = juce::Rectangle<float>(map.timeToX(b.start), map.area.getY(),
                                              map.timeToX(b.end) - map.timeToX(b.start), map.area.getHeight());
        g.setColour(Palette::window.withAlpha(0.75f));
        g.fillRoundedRectangle(r, 3.0f);
        g.setColour(Palette::window);
        g.drawRoundedRectangle(r, 3.0f, 1.0f);
    }

    g.setColour(Palette::dimText);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("FADER   drag: open | right-drag: cut | edges: resize | double-click: remove",
               getLocalBounds().reduced(16, 0).withHeight(14), juce::Justification::centredLeft);
}

void GateLaneView::mouseDown(const juce::MouseEvent& e)
{
    auto& lane = session.getPattern().getGate();
    const auto map = mapper();
    base = lane;
    dragged = false;
    blockIndex = -1;
    anchor = map.snap(map.xToTime(e.position.x), snapStep);

    if (e.mods.isPopupMenu())
    {
        mode = Mode::erase;
        return;
    }

    bool startEdge = false, endEdge = false;
    blockIndex = blockAt(base, e.position.x, startEdge, endEdge);
    if (blockIndex < 0)
    {
        mode = Mode::paint;
        return;
    }

    mode = startEdge ? Mode::resizeStart : endEdge ? Mode::resizeEnd : Mode::move;
    grabOffset = map.xToTime(e.position.x) - base.getBlocks()[(size_t) blockIndex].start;
}

void GateLaneView::mouseDrag(const juce::MouseEvent& e)
{
    if (mode == Mode::none)
        return;

    dragged = true;
    auto& lane = session.getPattern().getGate();
    const auto map = mapper();
    const double now = map.snap(map.xToTime(e.position.x), snapStep);
    lane = base;

    switch (mode)
    {
        case Mode::paint:
            lane.paint(juce::jmin(anchor, now), juce::jmax(anchor, now));
            break;
        case Mode::erase:
            lane.erase(juce::jmin(anchor, now), juce::jmax(anchor, now));
            break;
        case Mode::resizeStart:
        {
            const auto b = base.getBlocks()[(size_t) blockIndex];
            lane.setBlock(blockIndex, juce::jmin(now, b.end - minimumLength()), b.end);
            break;
        }
        case Mode::resizeEnd:
        {
            const auto b = base.getBlocks()[(size_t) blockIndex];
            lane.setBlock(blockIndex, b.start, juce::jmax(now, b.start + minimumLength()));
            break;
        }
        case Mode::move:
        {
            const auto b = base.getBlocks()[(size_t) blockIndex];
            const double length = b.end - b.start;
            const double start = juce::jlimit(0.0, juce::jmax(0.0, 1.0 - length),
                                              map.snap(map.xToTime(e.position.x) - grabOffset, snapStep));
            lane.setBlock(blockIndex, start, start + length);
            break;
        }
        case Mode::none:
            break;
    }

    lane.normalise();
    edited();
}

void GateLaneView::mouseUp(const juce::MouseEvent&)
{
    if (mode == Mode::paint && ! dragged)
    {
        // a plain click opens one snap step
        session.getPattern().getGate().paint(anchor, juce::jmin(1.0, anchor + minimumLength()));
        edited();
    }
    mode = Mode::none;
}

void GateLaneView::mouseDoubleClick(const juce::MouseEvent& e)
{
    auto& lane = session.getPattern().getGate();
    bool startEdge = false, endEdge = false;
    const int index = blockAt(lane, e.position.x, startEdge, endEdge);
    if (index >= 0)
    {
        mode = Mode::none;
        lane.removeBlock(index);
        edited();
    }
}
