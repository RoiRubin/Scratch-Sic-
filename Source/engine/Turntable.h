#pragma once
#include "Record.h"
#include "Platter.h"
#include "MotionGate.h"
#include "Crossfader.h"

// The deck. Given where the needle should be and where the fader is for each output sample, it
// moves the needle there like a heavy platter would and produces the scratched sound.
class Turntable
{
public:
    void prepare(double hostSampleRate, float platterWeight, double faderFadeSeconds);

    // Puts the needle down at startPosition (in record samples) with the fader at faderValue
    void start(const Record& record, double startPosition, float faderValue) noexcept;

    void step(const Record& record, double targetPosition, float faderValue, float& left, float& right) noexcept;

    double getNeedle() const noexcept { return needle; }

private:
    Platter platter;
    MotionGate gate;
    Crossfader crossfader;
    double hostRate = 44100.0;
    double needle = 0.0;
    double lastTarget = 0.0;
};
