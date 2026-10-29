#pragma once
#include "scratch/RenderedScratch.h"
#include <memory>

// Plays the rendered scratch through the plugin's output when the user presses Play.
// The editor starts and stops it; the audio thread only reads.
class PreviewPlayer
{
public:
    // Message thread. Swapping the sound mid-play keeps the play position.
    void setRendered(std::shared_ptr<const RenderedScratch> newRendered);
    std::shared_ptr<const RenderedScratch> getRendered() const;

    void play() noexcept;
    void stop() noexcept;
    void setLooping(bool shouldLoop) noexcept { looping.store(shouldLoop); }
    bool isPlaying() const noexcept { return playing.load(); }
    int getPositionSamples() const noexcept { return position.load(); }

    // Audio thread. Adds the scratch into the (already cleared) output.
    void process(juce::AudioBuffer<float>& output) noexcept;

private:
    std::shared_ptr<const RenderedScratch> rendered;
    mutable juce::SpinLock lock;
    std::atomic<bool> playing { false }, looping { false };
    std::atomic<int> position { 0 };
};
