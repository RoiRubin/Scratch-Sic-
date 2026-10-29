#include "Turntable.h"
#include "Hermite.h"
#include "Resampler.h"

namespace
{
    constexpr double skipThreshold = 500.0;   // record samples the target may move in one output sample
}

void Turntable::prepare(double hostSampleRate, float platterWeight, double faderFadeSeconds)
{
    hostRate = hostSampleRate;
    platter.prepare(hostSampleRate, platterWeight);
    gate.prepare(hostSampleRate);
    crossfader.prepare(hostSampleRate, faderFadeSeconds);
}

void Turntable::start(const Record& record, double startPosition, float faderValue) noexcept
{
    needle = lastTarget = startPosition;
    platter.reset(startPosition);
    gate.reset();
    crossfader.reset(faderValue);

    const auto& audio = record.getAudio();
    for (int ch = 0; ch < MotionGate::maxChannels; ++ch)
    {
        const float* data = audio.getReadPointer(juce::jmin(ch, audio.getNumChannels() - 1));
        gate.prime(ch, Hermite::interpolate(data, record.getNumSamples(), startPosition));
    }
}

void Turntable::step(const Record& record, double targetPosition, float faderValue, float& left, float& right) noexcept
{
    // A straight-up or straight-down line in the graph is the needle being lifted and put down somewhere
    // else, far faster than any hand could drag it. Land there at once instead of sweeping through the sample.
    const bool lifted = std::abs(targetPosition - lastTarget) > skipThreshold;
    lastTarget = targetPosition;
    if (lifted)
        platter.reset(targetPosition);

    const double position = lifted ? targetPosition : platter.process(targetPosition);
    const double speed = lifted ? 0.0 : position - needle;   // record samples per output sample
    needle = position;

    crossfader.setTarget(faderValue);
    const double relativeSpeed = std::abs(speed) * hostRate / record.getSampleRate();
    const float gain = gate.nextGain(relativeSpeed) * crossfader.nextGain();

    const auto& audio = record.getAudio();
    const int length = record.getNumSamples();
    const float* dataLeft = audio.getReadPointer(0);
    const float* dataRight = audio.getReadPointer(audio.getNumChannels() > 1 ? 1 : 0);

    left = gate.removeOffset(0, Resampler::read(dataLeft, length, position, speed)) * gain;
    right = gate.removeOffset(1, Resampler::read(dataRight, length, position, speed)) * gain;
}
