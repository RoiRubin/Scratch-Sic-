#include "PositionLaneView.h"

namespace
{
    constexpr float verticalPadding = 14.0f;
    constexpr float pointRadius = 6.0f, handleRadius = 4.5f, hitRadius = 10.0f;
    constexpr float curveDragPixelsForFullBend = 110.0f;
}

GridMapper PositionLaneView::mapper() const
{
    return { getLocalBounds(), verticalPadding, view };
}

juce::Point<float> PositionLaneView::handlePosition(const GridMapper& map, int segment) const
{
    const auto& lane = session.getPattern().getPosition();
    const auto& points = lane.getPoints();
    const double midTime = 0.5 * (points[(size_t) segment].time + points[(size_t) segment + 1].time);
    return { map.timeToX(midTime), map.valueToY(lane.valueInSegment(segment, 0.5)) };
}

int PositionLaneView::hitPoint(juce::Point<float> p) const
{
    const auto map = mapper();
    const auto& points = session.getPattern().getPosition().getPoints();
    for (int i = (int) points.size() - 1; i >= 0; --i)
        if (p.getDistanceFrom({ map.timeToX(points[(size_t) i].time), map.valueToY(points[(size_t) i].value) }) <= hitRadius)
            return i;
    return -1;
}

int PositionLaneView::hitHandle(juce::Point<float> p) const
{
    const auto map = mapper();
    const int segments = (int) session.getPattern().getPosition().getPoints().size() - 1;
    for (int i = 0; i < segments; ++i)
        if (p.getDistanceFrom(handlePosition(map, i)) <= hitRadius)
            return i;
    return -1;
}

bool PositionLaneView::isSelected(int index) const
{
    return std::binary_search(selected.begin(), selected.end(), index);
}

void PositionLaneView::toggleSelected(int index)
{
    const auto it = std::lower_bound(selected.begin(), selected.end(), index);
    if (it != selected.end() && *it == index)
        selected.erase(it);
    else
        selected.insert(it, index);
}

void PositionLaneView::pruneSelection()
{
    const int count = (int) session.getPattern().getPosition().getPoints().size();
    selected.erase(std::remove_if(selected.begin(), selected.end(), [count](int i) { return i >= count; }), selected.end());
}

void PositionLaneView::showSegmentMenu(int segment)
{
    const auto& points = session.getPattern().getPosition().getPoints();
    if (segment < 0 || segment + 1 >= (int) points.size())
        return;

    const bool smooth = points[(size_t) segment].smooth;
    juce::PopupMenu menu;
    menu.addItem(1, "Smooth", true, smooth);
    menu.addItem(2, "Straight", true, ! smooth);

    juce::Component::SafePointer<PositionLaneView> safeThis(this);
    menu.showMenuAsync(juce::PopupMenu::Options().withMousePosition(),
                       [safeThis, segment](int choice)
                       {
                           if (safeThis == nullptr || choice == 0)
                               return;
                           safeThis->session.getPattern().getPosition().setSegmentSmooth(segment, choice == 1);
                           safeThis->edited();
                       });
}

void PositionLaneView::clearSelection()
{
    selected.clear();
    mode = Mode::none;
    repaint();
}

void PositionLaneView::edited()
{
    session.patternEdited();
    repaint();
}

void PositionLaneView::paint(juce::Graphics& g)
{
    g.setColour(Palette::panel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

    const auto map = mapper();
    const auto& lane = session.getPattern().getPosition();
    const auto& points = lane.getPoints();

    drawTimeGrid(g, map, snapStep);

    g.setFont(juce::FontOptions(11.0f));
    for (int i = 0; i <= 4; ++i)
    {
        const float y = map.valueToY(i * 0.25f);
        g.setColour(Palette::outline.withAlpha(i == 0 || i == 4 ? 0.9f : 0.45f));
        g.drawHorizontalLine((int) y, map.area.getX(), map.area.getRight());
    }
    g.setColour(Palette::dimText);
    g.drawText("window end", getLocalBounds().withTrimmedLeft(16).withHeight(14), juce::Justification::centredRight);
    g.drawText("window start", getLocalBounds().withTrimmedLeft(16).removeFromBottom(14), juce::Justification::centredRight);
    g.drawText("RECORD POSITION", getLocalBounds().reduced(16, 0).withHeight(14), juce::Justification::centredLeft);
    if (selected.size() >= 2)
        g.drawText("Ctrl + drag an outer dot to tilt or stretch the selection",
                   getLocalBounds().withTrimmedLeft(150).withTrimmedRight(100).withHeight(14), juce::Justification::centred);

    juce::Path line;
    for (float x = map.area.getX(); x <= map.area.getRight(); x += 2.0f)
    {
        const float y = map.valueToY(lane.valueAt(map.xToTime(x)));
        x == map.area.getX() ? line.startNewSubPath(x, y) : line.lineTo(x, y);
    }
    line.lineTo(map.area.getRight(), map.valueToY(lane.valueAt(map.xToTime(map.area.getRight()))));

    juce::Path fill(line);
    fill.lineTo(map.area.getRight(), map.area.getBottom());
    fill.lineTo(map.area.getX(), map.area.getBottom());
    fill.closeSubPath();
    g.setColour(Palette::accent.withAlpha(0.12f));
    g.fillPath(fill);
    g.setColour(Palette::accent);
    g.strokePath(line, juce::PathStrokeType(2.0f));

    for (int i = 0; i + 1 < (int) points.size(); ++i)
    {
        if (points[(size_t) i + 1].time - points[(size_t) i].time < 1.0e-6)
            continue;
        const auto h = handlePosition(map, i);
        const bool active = i == hoverHandle || (mode == Mode::curve && i == dragIndex);
        g.setColour(active ? Palette::text : Palette::dimText);
        if (points[(size_t) i].smooth)
        {
            g.drawEllipse(juce::Rectangle<float>(handleRadius * 2, handleRadius * 2).withCentre(h), 1.5f);
        }
        else
        {
            const float d = handleRadius + 1.5f;   // a diamond marks a straight stretch
            juce::Path diamond;
            diamond.addQuadrilateral(h.x, h.y - d, h.x + d, h.y, h.x, h.y + d, h.x - d, h.y);
            g.strokePath(diamond, juce::PathStrokeType(1.5f));
        }
    }

    for (int i = 0; i < (int) points.size(); ++i)
    {
        const juce::Point<float> c { map.timeToX(points[(size_t) i].time), map.valueToY(points[(size_t) i].value) };
        const bool selectedNow = isSelected(i);
        const bool outerEnd = selected.size() >= 2 && (i == selected.front() || i == selected.back());
        const float r = (i == hoverPoint || (mode != Mode::none && i == dragIndex)) ? pointRadius + 2.0f : pointRadius;

        if (outerEnd)
        {
            g.setColour(Palette::window);
            g.drawEllipse(juce::Rectangle<float>((r + 5) * 2, (r + 5) * 2).withCentre(c), 2.0f);
        }
        g.setColour(selectedNow ? Palette::text : Palette::accent);
        g.fillEllipse(juce::Rectangle<float>(r * 2, r * 2).withCentre(c));
        g.setColour(selectedNow ? Palette::accent : Palette::text);
        g.drawEllipse(juce::Rectangle<float>(r * 2, r * 2).withCentre(c), 1.5f);
    }

    if (mode == Mode::marquee)
    {
        g.setColour(Palette::window.withAlpha(0.15f));
        g.fillRect(marquee);
        g.setColour(Palette::window);
        g.drawRect(marquee, 1.0f);
    }
}

void PositionLaneView::mouseDown(const juce::MouseEvent& e)
{
    auto& lane = session.getPattern().getPosition();
    pruneSelection();
    const int p = hitPoint(e.position);

    if (e.mods.isPopupMenu())
    {
        if (p >= 0)
        {
            // right-clicking a selected point removes the whole selection
            if (isSelected(p) && selected.size() > 1)
                lane.removePoints(selected);
            else
                lane.removePoint(p);
            selected.clear();
            edited();
        }
        else if (const int h = hitHandle(e.position); h >= 0)
        {
            showSegmentMenu(h);
        }
        return;
    }

    if (e.mods.isShiftDown())
    {
        if (p >= 0)
        {
            toggleSelected(p);
        }
        else
        {
            mode = Mode::marquee;   // adds to whatever is already selected
            marqueeStart = e.position;
            marquee = {};
            selectedBeforeMarquee = selected;
        }
        repaint();
        return;
    }

    if (p >= 0)
    {
        if (! isSelected(p))
            selected = { p };

        dragIndex = p;
        dragOriginal = lane.getPoints();
        const auto map = mapper();
        grabOffset = e.position - juce::Point<float>(map.timeToX(dragOriginal[(size_t) p].time), map.valueToY(dragOriginal[(size_t) p].value));
        stretchLastEnd = p == selected.back();
        const bool outerEnd = selected.size() >= 2 && (p == selected.front() || p == selected.back());
        mode = e.mods.isCommandDown() && outerEnd ? Mode::stretch : Mode::move;
        repaint();
        return;
    }

    if (const int h = hitHandle(e.position); h >= 0)
    {
        mode = Mode::curve;
        dragIndex = h;
        curveDragStartY = e.position.y;
        curveDragStartValue = lane.getPoints()[(size_t) h].curve;
        return;
    }

    const auto map = mapper();
    dragIndex = lane.addPoint(map.snap(map.xToTime(e.position.x), snapStep), map.yToValue(e.position.y));
    selected = { dragIndex };
    dragOriginal = lane.getPoints();
    grabOffset = {};
    mode = Mode::move;
    edited();
}

void PositionLaneView::mouseDrag(const juce::MouseEvent& e)
{
    auto& lane = session.getPattern().getPosition();
    const auto& points = lane.getPoints();

    if (mode == Mode::marquee)
    {
        marquee = juce::Rectangle<float>(marqueeStart, e.position);
        selected = selectedBeforeMarquee;

        const auto map = mapper();
        for (int i = 0; i < (int) points.size(); ++i)
            if (marquee.contains(juce::Point<float>(map.timeToX(points[(size_t) i].time), map.valueToY(points[(size_t) i].value))))
                selected.push_back(i);

        std::sort(selected.begin(), selected.end());
        selected.erase(std::unique(selected.begin(), selected.end()), selected.end());
        repaint();
        return;
    }

    if (dragIndex < 0 || dragIndex >= (int) points.size())
        return;

    if (mode == Mode::move || mode == Mode::stretch)
    {
        if (dragOriginal.size() != points.size())
            return;

        // the grabbed point follows the pointer; everything else follows the grabbed point
        const auto map = mapper();
        const auto target = e.position - grabOffset;
        const double time = map.snap(map.xToTime(target.x), snapStep);
        const float value = map.yToValue(target.y);
        if (mode == Mode::move)
            lane.moveSelection(dragOriginal, selected, time - dragOriginal[(size_t) dragIndex].time,
                               value - dragOriginal[(size_t) dragIndex].value);
        else
            lane.stretchSelection(dragOriginal, selected, stretchLastEnd, time, value);
        edited();
    }
    else if (mode == Mode::curve && dragIndex + 1 < (int) points.size())
    {
        // dragging down always bows the line downward, whichever way the line slopes
        const float direction = points[(size_t) dragIndex + 1].value > points[(size_t) dragIndex].value ? 1.0f : -1.0f;
        lane.setCurve(dragIndex, curveDragStartValue + direction * (e.position.y - curveDragStartY) / curveDragPixelsForFullBend);
        edited();
    }
}

void PositionLaneView::mouseUp(const juce::MouseEvent&)
{
    mode = Mode::none;
    dragIndex = -1;
    marquee = {};
    repaint();
}

void PositionLaneView::mouseDoubleClick(const juce::MouseEvent& e)
{
    auto& lane = session.getPattern().getPosition();
    if (const int p = hitPoint(e.position); p >= 0)
    {
        mode = Mode::none;
        dragIndex = -1;
        selected.clear();
        lane.removePoint(p);
        edited();
    }
    else if (const int h = hitHandle(e.position); h >= 0)
    {
        lane.setCurve(h, 0.0f);
        edited();
    }
}

void PositionLaneView::mouseMove(const juce::MouseEvent& e)
{
    const int p = hitPoint(e.position);
    const int h = p >= 0 ? -1 : hitHandle(e.position);
    if (p != hoverPoint || h != hoverHandle)
    {
        hoverPoint = p;
        hoverHandle = h;
        repaint();
    }
}

void PositionLaneView::mouseExit(const juce::MouseEvent&)
{
    hoverPoint = hoverHandle = -1;
    repaint();
}
