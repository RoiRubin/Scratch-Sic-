#include "ScratchSession.h"
#include "PluginProcessor.h"
#include "ScratchRenderer.h"
#include "ScratchExporter.h"

namespace
{
    constexpr int checkIntervalMs = 30;
}

bool ScratchSession::RenderInputs::operator==(const RenderInputs& o) const noexcept
{
    return recordId == o.recordId
        && juce::approximatelyEqual(windowStart, o.windowStart)
        && juce::approximatelyEqual(windowLength, o.windowLength)
        && juce::approximatelyEqual(platterWeight, o.platterWeight)
        && juce::approximatelyEqual(faderSmooth, o.faderSmooth)
        && juce::approximatelyEqual(sampleRate, o.sampleRate)
        && fullSample == o.fullSample;
}

ScratchSession::ScratchSession(ScratchProcessor& processor)
    : owner(processor)
{
    serialised = pattern.toXmlString();
    startTimer(checkIntervalMs);
}

ScratchSession::~ScratchSession()
{
    stopTimer();
}

void ScratchSession::patternEdited()
{
    dirty = true;
    const juce::ScopedLock lock(serialisedLock);
    serialised = pattern.toXmlString();
}

void ScratchSession::loadPattern(const ScratchPattern& p)
{
    pattern = p;
    patternEdited();
    sendChangeMessage();
}

juce::String ScratchSession::getSerialisedPattern() const
{
    const juce::ScopedLock lock(serialisedLock);
    return serialised;
}

void ScratchSession::restoreSerialisedPattern(const juce::String& xml)
{
    if (xml.isEmpty())
        return;
    if (auto restored = ScratchPattern::fromXmlString(xml))
        loadPattern(*restored);
}

bool ScratchSession::hasRendered() const
{
    return owner.getPreview().getRendered() != nullptr;
}

juce::File ScratchSession::exportCurrent()
{
    const auto rendered = owner.getPreview().getRendered();
    if (rendered == nullptr)
        return {};
    return ScratchExporter::writeWav(*rendered, owner.getParams().outputGain.load());
}

void ScratchSession::timerCallback()
{
    const auto record = owner.getRecord();
    const double sampleRate = owner.getCurrentSampleRate();

    if (record == nullptr || sampleRate <= 0.0)
    {
        if (hasSound)
        {
            owner.getPreview().setRendered(nullptr);
            hasSound = false;
            lastInputs = {};
        }
        return;
    }

    const auto& params = owner.getParams();
    const RenderInputs now { record->getId(), params.windowStart.load(), params.windowLength.load(),
                             params.platterWeight.load(), params.faderSmooth.load(), sampleRate,
                             params.fullSample.load() >= 0.5f };
    if (! dirty && now == lastInputs)
        return;

    dirty = false;
    lastInputs = now;

    RenderSettings settings;
    settings.sampleRate = now.sampleRate;
    settings.windowStart = now.windowStart;
    settings.windowLengthSeconds = now.windowLength;
    settings.platterWeight = now.platterWeight;
    settings.faderSmoothMs = now.faderSmooth;
    settings.includeSurround = now.fullSample;

    owner.getPreview().setRendered(ScratchRenderer::render(record, pattern, settings));
    hasSound = true;
}
