#pragma once

// The DJ's fader: cuts the sound in and out. Each cut fades along a smooth S-shaped curve
// (slow start, slow finish) instead of a straight ramp, which keeps cuts from sounding hard or clicky.
class Crossfader
{
public:
    void prepare(double sampleRate, double fadeSeconds);
    void reset(float value) noexcept;

    void setTarget(float faderValue) noexcept;   // 1 = open, 0 = cut
    float nextGain() noexcept;

private:
    float progress = 0.0f, target = 0.0f, step = 1.0f;
};
