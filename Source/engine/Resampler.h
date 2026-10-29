#pragma once
#include "Hermite.h"
#include <algorithm>
#include <cmath>

// Reads the record at a fractional position. When the needle moves faster than the sample's own speed
// it covers several samples per output sample, which makes harsh whistling "aliasing". Averaging a few
// readings along the path the needle travelled filters that out.
namespace Resampler
{
    constexpr int maxTaps = 24;

    // speed: how many record samples the needle moves per output sample
    inline float read(const float* data, int numSamples, double position, double speed) noexcept
    {
        const int taps = std::clamp((int) std::ceil(std::abs(speed)), 1, maxTaps);
        if (taps == 1)
            return Hermite::interpolate(data, numSamples, position);

        float sum = 0.0f;
        for (int k = 0; k < taps; ++k)
        {
            const double offset = ((k + 0.5) / taps - 0.5) * speed;
            sum += Hermite::interpolate(data, numSamples, position + offset);
        }
        return sum / (float) taps;
    }
}
