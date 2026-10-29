#include "MotionGate.h"
#include <cmath>
#include <algorithm>

namespace
{
    constexpr double fullVolumeSpeed = 0.05;   // at 5% of normal speed and above, the sound is fully open
    constexpr double fadeSeconds = 0.002;
    constexpr double offsetCutoffHz = 5.0;   // low enough to leave slow, deep scratches alone
    constexpr double twoPi = 6.283185307179586;
}

void MotionGate::prepare(double sampleRate)
{
    gainCoeff = (float) (1.0 - std::exp(-1.0 / (fadeSeconds * sampleRate)));
    blockerCoeff = (float) std::exp(-twoPi * offsetCutoffHz / sampleRate);
    reset();
}

void MotionGate::reset() noexcept
{
    gain = 0.0f;
    lastIn.fill(0.0f);
    lastOut.fill(0.0f);
}

void MotionGate::prime(int channel, float value) noexcept
{
    lastIn[(size_t) channel] = value;
    lastOut[(size_t) channel] = 0.0f;
}

float MotionGate::nextGain(double speed) noexcept
{
    const auto targetGain = (float) std::min(1.0, std::abs(speed) / fullVolumeSpeed);
    gain += gainCoeff * (targetGain - gain);
    return gain;
}

float MotionGate::removeOffset(int channel, float input) noexcept
{
    const float output = input - lastIn[(size_t) channel] + blockerCoeff * lastOut[(size_t) channel];
    lastIn[(size_t) channel] = input;
    lastOut[(size_t) channel] = output;
    return output;
}
