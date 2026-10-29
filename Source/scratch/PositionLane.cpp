#include "PositionLane.h"
#include <algorithm>
#include <cmath>
#include <functional>

namespace
{
    constexpr double curveStrength = 4.0;   // curve = +-1 makes the line's exponent 4 or 1/4
    constexpr double tinySpan = 1.0e-9;
}

PositionLane::PositionLane()
    : points { { 0.0, 0.0f, 0.0f }, { 1.0, 1.0f, 0.0f } }
{
    updateSlopes();
}

PositionLane::PositionLane(std::vector<PositionPoint> newPoints)
    : points(std::move(newPoints))
{
    std::stable_sort(points.begin(), points.end(), [](const auto& a, const auto& b) { return a.time < b.time; });
    if (points.size() < 2)
        *this = PositionLane();
    updateSlopes();
}

bool PositionLane::areAllSmooth() const noexcept
{
    for (size_t i = 0; i + 1 < points.size(); ++i)
        if (! points[i].smooth)
            return false;
    return true;
}

void PositionLane::setAllSmooth(bool shouldSmooth)
{
    for (auto& p : points)
        p.smooth = shouldSmooth;
}

void PositionLane::setSegmentSmooth(int index, bool shouldSmooth)
{
    if (index >= 0 && index + 1 < (int) points.size())
        points[(size_t) index].smooth = shouldSmooth;
}

// Monotone cubic (Fritsch-Carlson): flows through every point without overshooting,
// and is flat at peaks and dips, which is where a hand turns the record round.
void PositionLane::updateSlopes()
{
    const size_t n = points.size();
    slopes.assign(n, 0.0);
    if (n < 2)
        return;

    std::vector<double> span(n - 1), secant(n - 1);
    for (size_t k = 0; k + 1 < n; ++k)
    {
        span[k] = points[k + 1].time - points[k].time;
        secant[k] = span[k] > tinySpan ? (points[k + 1].value - points[k].value) / span[k] : 0.0;
    }

    slopes[0] = secant[0];
    slopes[n - 1] = secant[n - 2];
    for (size_t k = 1; k + 1 < n; ++k)
    {
        if (secant[k - 1] * secant[k] <= 0.0 || span[k - 1] <= tinySpan || span[k] <= tinySpan)
            continue;   // a peak, a dip, or a straight jump: flat here

        const double w1 = 2.0 * span[k] + span[k - 1], w2 = span[k] + 2.0 * span[k - 1];
        slopes[k] = (w1 + w2) / (w1 / secant[k - 1] + w2 / secant[k]);
    }
}

float PositionLane::valueInSegment(int index, double t) const noexcept
{
    const auto& a = points[(size_t) index];
    const auto& b = points[(size_t) index + 1];
    const double span = b.time - a.time;
    const double u = std::pow(juce::jlimit(0.0, 1.0, t), std::pow(curveStrength, (double) a.curve));

    if (! a.smooth || span <= tinySpan)
        return a.value + (b.value - a.value) * (float) u;

    const double u2 = u * u, u3 = u2 * u;
    const double v = (2 * u3 - 3 * u2 + 1) * a.value + (u3 - 2 * u2 + u) * span * slopes[(size_t) index]
                   + (-2 * u3 + 3 * u2) * b.value + (u3 - u2) * span * slopes[(size_t) index + 1];
    return juce::jlimit(0.0f, 1.0f, (float) v);
}

float PositionLane::valueAt(double time) const noexcept
{
    if (time <= points.front().time) return points.front().value;
    if (time >= points.back().time) return points.back().value;

    const auto next = std::upper_bound(points.begin(), points.end(), time,
                                       [](double t, const PositionPoint& p) { return t < p.time; });
    const int index = (int) (next - points.begin()) - 1;
    const double span = points[(size_t) index + 1].time - points[(size_t) index].time;
    const double t = span > tinySpan ? (time - points[(size_t) index].time) / span : 1.0;
    return valueInSegment(index, t);
}

int PositionLane::addPoint(double time, float value)
{
    const auto next = std::upper_bound(points.begin(), points.end(), time,
                                       [](double t, const PositionPoint& p) { return t < p.time; });
    // a point added on a line keeps that line's character
    const bool inherited = next != points.begin() ? (next - 1)->smooth : next->smooth;
    const auto inserted = points.insert(next, { time, juce::jlimit(0.0f, 1.0f, value), 0.0f, inherited });
    updateSlopes();
    return (int) (inserted - points.begin());
}

namespace
{
    // For every point: the time of the nearest unselected point on its left / right (or the edge of the graph)
    struct Room
    {
        std::vector<double> low, high;

        Room(const std::vector<PositionPoint>& original, const std::vector<int>& selected)
            : low(original.size(), 0.0), high(original.size(), 1.0)
        {
            const size_t n = original.size();
            std::vector<bool> isSelected(n, false);
            for (int i : selected)
                isSelected[(size_t) i] = true;

            for (size_t i = 1; i < n; ++i)
                low[i] = isSelected[i - 1] ? low[i - 1] : original[i - 1].time;
            for (size_t i = n - 1; i-- > 0;)
                high[i] = isSelected[i + 1] ? high[i + 1] : original[i + 1].time;
        }
    };
}

void PositionLane::moveSelection(const std::vector<PositionPoint>& original, const std::vector<int>& selected,
                                 double deltaTime, float deltaValue)
{
    if (selected.empty() || original.size() != points.size())
        return;

    const Room room(original, selected);
    double earliest = -1.0, latest = 1.0;
    float lowest = -1.0f, highest = 1.0f;
    for (int i : selected)
    {
        const auto& p = original[(size_t) i];
        earliest = juce::jmax(earliest, room.low[(size_t) i] - p.time);
        latest = juce::jmin(latest, room.high[(size_t) i] - p.time);
        lowest = juce::jmax(lowest, -p.value);
        highest = juce::jmin(highest, 1.0f - p.value);
    }

    const double dt = juce::jlimit(earliest, juce::jmax(earliest, latest), deltaTime);
    const float dv = juce::jlimit(lowest, juce::jmax(lowest, highest), deltaValue);
    for (int i : selected)
    {
        points[(size_t) i].time = original[(size_t) i].time + dt;
        points[(size_t) i].value = original[(size_t) i].value + dv;
    }
    updateSlopes();
}

void PositionLane::stretchSelection(const std::vector<PositionPoint>& original, const std::vector<int>& selected,
                                    bool draggingLastEnd, double newTime, float newValue)
{
    if (selected.size() < 2 || original.size() != points.size())
        return;

    const Room room(original, selected);
    const auto& first = original[(size_t) selected.front()];
    const auto& last = original[(size_t) selected.back()];
    const auto& dragged = draggingLastEnd ? last : first;
    const auto& fixed = draggingLastEnd ? first : last;
    const double span = last.time - first.time;
    const double lastRank = (double) (selected.size() - 1);

    // how far along each point is, from 0 at the end that stays put to 1 at the end being dragged
    const auto along = [&](size_t rank, int index)
    {
        const double fromFirst = span > tinySpan ? (original[(size_t) index].time - first.time) / span : (double) rank / lastRank;
        return draggingLastEnd ? fromFirst : 1.0 - fromFirst;
    };

    // tilt: every point shifts up or down in proportion to how far along it is; shortened if one would leave the graph
    double lowest = -1.0, highest = 1.0;
    for (size_t r = 0; r < selected.size(); ++r)
    {
        const double u = along(r, selected[r]);
        if (u <= tinySpan)
            continue;
        const double v = original[(size_t) selected[r]].value;
        lowest = juce::jmax(lowest, -v / u);
        highest = juce::jmin(highest, (1.0 - v) / u);
    }
    const double tilt = juce::jlimit(lowest, juce::jmax(lowest, highest), (double) newValue - dragged.value);

    // stretch: times scale about the end that stays put
    const double scale = span > tinySpan
        ? juce::jmax(0.0, (draggingLastEnd ? newTime - fixed.time : fixed.time - newTime) / span)
        : 1.0;

    for (size_t r = 0; r < selected.size(); ++r)
    {
        const auto i = (size_t) selected[r];
        points[i].value = juce::jlimit(0.0f, 1.0f, (float) (original[i].value + along(r, selected[r]) * tilt));
        points[i].time = juce::jlimit(room.low[i], juce::jmax(room.low[i], room.high[i]),
                                      fixed.time + (original[i].time - fixed.time) * scale);
    }
    updateSlopes();
}

void PositionLane::removePoints(const std::vector<int>& indices)
{
    auto descending = indices;
    std::sort(descending.begin(), descending.end(), std::greater<int>());
    descending.erase(std::unique(descending.begin(), descending.end()), descending.end());

    for (int i : descending)
        if (points.size() > 2 && i >= 0 && i < (int) points.size())
            points.erase(points.begin() + i);
    updateSlopes();
}

void PositionLane::removePoint(int index)
{
    if (points.size() > 2 && index >= 0 && index < (int) points.size())
    {
        points.erase(points.begin() + index);
        updateSlopes();
    }
}

void PositionLane::setCurve(int index, float curve)
{
    if (index >= 0 && index + 1 < (int) points.size())
        points[(size_t) index].curve = juce::jlimit(-1.0f, 1.0f, curve);
}

void PositionLane::scaleTime(double factor)
{
    for (auto& p : points)
        p.time = juce::jlimit(0.0, 1.0, p.time * factor);
    updateSlopes();
}

juce::ValueTree PositionLane::toValueTree() const
{
    juce::ValueTree tree("Position");
    for (const auto& p : points)
    {
        juce::ValueTree node("Point");
        node.setProperty("time", p.time, nullptr);
        node.setProperty("value", (double) p.value, nullptr);
        node.setProperty("curve", (double) p.curve, nullptr);
        node.setProperty("smooth", p.smooth, nullptr);
        tree.appendChild(node, nullptr);
    }
    return tree;
}

PositionLane PositionLane::fromValueTree(const juce::ValueTree& tree)
{
    // versions 0.3 and 0.4 kept one smooth setting for the whole line
    const bool wholeLineSmooth = tree.getProperty("smooth", true);

    std::vector<PositionPoint> loaded;
    for (const auto& node : tree)
    {
        // version 0.2 stored the time as "beat"
        const double time = node.hasProperty("time") ? (double) node.getProperty("time") : (double) node.getProperty("beat");
        loaded.push_back({ time,
                           juce::jlimit(0.0f, 1.0f, (float) (double) node.getProperty("value")),
                           juce::jlimit(-1.0f, 1.0f, (float) (double) node.getProperty("curve")),
                           (bool) node.getProperty("smooth", wholeLineSmooth) });
    }
    return PositionLane(std::move(loaded));
}
