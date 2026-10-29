#pragma once
#include <juce_core/juce_core.h>
#include <cmath>

// Reads a sample value at a fractional position by curving smoothly through the four nearest points.
// Keeps slow and odd-speed scratches clean instead of crackly.
namespace Hermite
{
    inline float interpolate(const float* data, int numSamples, double position) noexcept
    {
        const int i = (int) std::floor(position);
        const float t = (float) (position - i);
        const auto at = [data, numSamples](int k) { return data[juce::jlimit(0, numSamples - 1, k)]; };

        const float xm1 = at(i - 1), x0 = at(i), x1 = at(i + 1), x2 = at(i + 2);
        const float c1 = 0.5f * (x1 - xm1);
        const float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
        const float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
        return ((c3 * t + c2) * t + c1) * t + x0;
    }
}
