#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "engine/Record.h"
#include <memory>
#include <vector>

// A scratch turned into sound, optionally with the untouched parts of the sample before and after it.
// Never changes after it is made, so the audio thread can play it while the editor reads it.
//
// The scratch itself is stored as audio. The parts around it are not copied: they are read straight
// from the record, at normal speed, whenever they are played or exported.
struct RenderedScratch
{
    juce::AudioBuffer<float> scratch;  // stereo, at the host's sample rate
    std::vector<float> needle;         // where the needle is on the record (in record samples) for each scratch sample
    double sampleRate = 44100.0;
    double lengthSeconds = 1.0;        // the scratch alone: as long as the sample window
    juce::String patternName;

    std::shared_ptr<const Record> surround;   // set when the whole sample is included
    double windowStart = 0.0, windowEnd = 0.0; // the scratched part of the record, in record samples
    int beforeCount = 0, afterCount = 0;       // output samples of untouched audio before / after the scratch
    int fadeSamples = 1;                       // short fade at each join, so the joins don't click

    bool hasSurround() const noexcept { return surround != nullptr; }
    int getNumSamples() const noexcept { return beforeCount + scratch.getNumSamples() + afterCount; }
    int getScratchStart() const noexcept { return beforeCount; }
    int getScratchLength() const noexcept { return scratch.getNumSamples(); }

    float getSample(int channel, int position) const noexcept;
    double getNeedle(int position) const noexcept;   // in record samples
    juce::AudioBuffer<float> toBuffer() const;       // everything joined up, for export
};
