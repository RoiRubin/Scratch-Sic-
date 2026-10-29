#pragma once
#include "ScratchPattern.h"
#include "PatternBank.h"
#include "RenderedScratch.h"
#include <juce_events/juce_events.h>

class ScratchProcessor;

// Ties the pattern being edited to the sound: whenever the pattern, the sample, the window, the platter
// weight or fader smoothness changes, it renders the scratch again and hands it to the preview player.
// Lives on the message thread.
class ScratchSession : public juce::ChangeBroadcaster, private juce::Timer
{
public:
    explicit ScratchSession(ScratchProcessor& owner);
    ~ScratchSession() override;

    ScratchPattern& getPattern() noexcept { return pattern; }
    PatternBank& getBank() noexcept { return bank; }

    void patternEdited();                       // call after changing the pattern in place
    void loadPattern(const ScratchPattern& p);  // replace the pattern; listeners are told

    // Saved with the FL project. Safe to call from any thread.
    juce::String getSerialisedPattern() const;
    void restoreSerialisedPattern(const juce::String& xml);

    // Writes the current scratch to a WAV; returns a non-existent file when there is nothing to export
    juce::File exportCurrent();
    bool hasRendered() const;

private:
    struct RenderInputs
    {
        juce::uint32 recordId = 0;
        float windowStart = 0.0f, windowLength = 0.0f, platterWeight = 0.0f, faderSmooth = 0.0f;
        double sampleRate = 0.0;
        bool fullSample = true;
        bool operator==(const RenderInputs& o) const noexcept;
    };

    void timerCallback() override;

    ScratchProcessor& owner;
    ScratchPattern pattern;
    PatternBank bank;

    bool dirty = true;
    bool hasSound = false;
    RenderInputs lastInputs;

    mutable juce::CriticalSection serialisedLock;
    juce::String serialised;
};
