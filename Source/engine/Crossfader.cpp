#include "Crossfader.h"
#include <juce_core/juce_core.h>
#include <cmath>

void Crossfader::prepare(double sampleRate, double fadeSeconds)
{
    step = 1.0f / (float) juce::jmax(1.0, fadeSeconds * sampleRate);
}

void Crossfader::reset(float value) noexcept
{
    target = progress = value >= 0.5f ? 1.0f : 0.0f;
}

void Crossfader::setTarget(float faderValue) noexcept
{
    target = faderValue >= 0.5f ? 1.0f : 0.0f;
}

float Crossfader::nextGain() noexcept
{
    // a fade interrupted halfway simply turns round from where it is, so it never jumps
    if (progress < target)
        progress = juce::jmin(target, progress + step);
    else if (progress > target)
        progress = juce::jmax(target, progress - step);

    return 0.5f - 0.5f * std::cos(juce::MathConstants<float>::pi * progress);
}
