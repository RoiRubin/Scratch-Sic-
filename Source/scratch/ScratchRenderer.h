#pragma once
#include "ScratchPattern.h"
#include "RenderedScratch.h"
#include "engine/Record.h"

struct RenderSettings
{
    double sampleRate = 44100.0;
    float windowStart = 0.0f;           // 0..1 of the sample
    float windowLengthSeconds = 1.0f;
    float platterWeight = 0.3f;         // 0..1
    float faderSmoothMs = 4.0f;
    bool includeSurround = true;        // also play / export the untouched parts of the sample around the scratch
};

// Plays a pattern on a record, offline, and returns the sound. The scratch lasts exactly as long as the window.
class ScratchRenderer
{
public:
    static std::shared_ptr<const RenderedScratch> render(std::shared_ptr<const Record> record, const ScratchPattern& pattern,
                                                         const RenderSettings& settings);
};
