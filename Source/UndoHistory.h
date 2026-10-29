#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class ScratchProcessor;

// Ctrl+Z / Ctrl+Y for everything in the plugin: the scratch graphs, the knobs, the loaded sample and
// the sample window. It watches the plugin's state about 30 times a second and files the state from
// before each burst of edits; a burst ends after a short pause, so dragging a point or turning a knob
// is a single step. Lives on the message thread.
class UndoHistory : private juce::Timer
{
public:
    explicit UndoHistory(ScratchProcessor& owner);
    ~UndoHistory() override;

    void reset();   // forget everything and take the current state as the starting point

    bool undo();
    bool redo();
    bool canUndo() const noexcept { return burstOpen || ! undoStack.empty(); }
    bool canRedo() const noexcept { return ! redoStack.empty(); }

private:
    struct Snapshot
    {
        juce::String pattern, recordPath;
        std::vector<float> params;
        bool operator==(const Snapshot& other) const noexcept;
    };

    Snapshot capture() const;
    void restore(const Snapshot& snapshot);
    void timerCallback() override;
    void closeBurst();

    ScratchProcessor& owner;
    std::vector<Snapshot> undoStack, redoStack;
    Snapshot stable;      // the last state that has been filed
    Snapshot latest;      // the state as of the last check
    bool burstOpen = false;
    juce::uint32 lastChangeMs = 0;
};
