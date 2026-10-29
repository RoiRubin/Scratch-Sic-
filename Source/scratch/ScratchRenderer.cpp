#include "ScratchRenderer.h"
#include "engine/Turntable.h"

namespace
{
    constexpr double joinFadeSeconds = 0.0015;
}

std::shared_ptr<const RenderedScratch> ScratchRenderer::render(std::shared_ptr<const Record> record, const ScratchPattern& pattern,
                                                               const RenderSettings& settings)
{
    const auto window = record->windowFor(settings.windowStart, settings.windowLengthSeconds);
    const double seconds = window.getLength() / record->getSampleRate();
    const int numSamples = juce::jmax(1, (int) std::ceil(seconds * settings.sampleRate));

    auto result = std::make_shared<RenderedScratch>();
    result->scratch.setSize(2, numSamples);
    result->needle.resize((size_t) numSamples);
    result->sampleRate = settings.sampleRate;
    result->lengthSeconds = seconds;
    result->patternName = pattern.getName();

    const auto& positionLane = pattern.getPosition();
    const auto& gateLane = pattern.getGate();
    const auto targetAt = [&](double time) { return window.getStart() + (double) positionLane.valueAt(time) * window.getLength(); };
    const auto faderAt = [&](double time) { return gateLane.isOpenAt(time) ? 1.0f : 0.0f; };

    Turntable turntable;
    turntable.prepare(settings.sampleRate, settings.platterWeight, settings.faderSmoothMs * 0.001);
    turntable.start(*record, targetAt(0.0), faderAt(0.0));

    float* left = result->scratch.getWritePointer(0);
    float* right = result->scratch.getWritePointer(1);
    for (int n = 0; n < numSamples; ++n)
    {
        const double time = (double) n / numSamples;   // 0..1 across the graph
        turntable.step(*record, targetAt(time), faderAt(time), left[n], right[n]);
        result->needle[(size_t) n] = (float) turntable.getNeedle();
    }

    if (settings.includeSurround)
    {
        const double ratio = record->getSampleRate() / settings.sampleRate;
        result->surround = record;
        result->windowStart = window.getStart();
        result->windowEnd = window.getEnd();
        result->beforeCount = (int) std::floor(window.getStart() / ratio);
        result->afterCount = (int) std::floor(((double) record->getNumSamples() - window.getEnd()) / ratio);
        result->fadeSamples = juce::jmax(1, (int) (joinFadeSeconds * settings.sampleRate));

        // let the scratch fade out into the untouched audio that follows it
        for (int i = 0; i < juce::jmin(result->fadeSamples, numSamples); ++i)
        {
            const float x = juce::jmin(1.0f, (float) (i + 1) / (float) result->fadeSamples);
            const float gain = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::pi * x);
            left[numSamples - 1 - i] *= gain;
            right[numSamples - 1 - i] *= gain;
        }
    }
    return result;
}
