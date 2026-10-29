#pragma once
#include <array>

// A real record makes no sound when it isn't moving. This fades the sound out as the needle slows
// to a stop, and removes the steady offset left behind when the needle rests on a non-zero sample.
class MotionGate
{
public:
    static constexpr int maxChannels = 2;

    void prepare(double sampleRate);
    void reset() noexcept;

    // Start a channel's offset remover at the sound's current level, so the start doesn't click
    void prime(int channel, float value) noexcept;

    // speed is the needle's speed relative to normal playback (1 = normal, 0 = stopped)
    float nextGain(double speed) noexcept;
    float removeOffset(int channel, float input) noexcept;

private:
    float gain = 0.0f, gainCoeff = 1.0f, blockerCoeff = 0.995f;
    std::array<float, maxChannels> lastIn {}, lastOut {};
};
