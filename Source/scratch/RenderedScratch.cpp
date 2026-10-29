#include "RenderedScratch.h"
#include "engine/Hermite.h"

namespace
{
    // 0 right at the join, rising smoothly to 1 once fade samples away
    float joinGain(int distanceFromJoin, int fade) noexcept
    {
        const float x = juce::jmin(1.0f, (float) distanceFromJoin / (float) fade);
        return 0.5f - 0.5f * std::cos(juce::MathConstants<float>::pi * x);
    }
}

float RenderedScratch::getSample(int channel, int position) const noexcept
{
    const int scratchLength = scratch.getNumSamples();
    if (position >= beforeCount && position < beforeCount + scratchLength)
        return scratch.getSample(juce::jmin(channel, scratch.getNumChannels() - 1), position - beforeCount);

    if (surround == nullptr)
        return 0.0f;

    const auto& audio = surround->getAudio();
    const float* data = audio.getReadPointer(juce::jmin(channel, audio.getNumChannels() - 1));
    const int length = surround->getNumSamples();
    const double ratio = surround->getSampleRate() / sampleRate;

    if (position < beforeCount)
        return Hermite::interpolate(data, length, position * ratio) * joinGain(beforeCount - position, fadeSamples);

    const int offset = position - beforeCount - scratchLength;
    return Hermite::interpolate(data, length, windowEnd + offset * ratio) * joinGain(offset, fadeSamples);
}

double RenderedScratch::getNeedle(int position) const noexcept
{
    const int scratchLength = scratch.getNumSamples();
    if (position >= beforeCount && position < beforeCount + scratchLength)
        return needle[(size_t) (position - beforeCount)];

    if (surround == nullptr)
        return needle.empty() ? 0.0 : needle.front();

    const double ratio = surround->getSampleRate() / sampleRate;
    if (position < beforeCount)
        return position * ratio;
    return windowEnd + (position - beforeCount - scratchLength) * ratio;
}

juce::AudioBuffer<float> RenderedScratch::toBuffer() const
{
    const int total = getNumSamples();
    juce::AudioBuffer<float> joined(2, total);
    for (int ch = 0; ch < 2; ++ch)
    {
        float* out = joined.getWritePointer(ch);
        for (int n = 0; n < total; ++n)
            out[n] = getSample(ch, n);
    }
    return joined;
}
