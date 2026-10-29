#include "UndoHistory.h"
#include "PluginProcessor.h"

namespace
{
    constexpr int checkHz = 30;
    constexpr juce::uint32 quietMs = 400;   // a burst of edits ends after this long without a change
    constexpr size_t maxSteps = 200;
}

bool UndoHistory::Snapshot::operator==(const Snapshot& other) const noexcept
{
    return pattern == other.pattern && recordPath == other.recordPath && params == other.params;
}

UndoHistory::UndoHistory(ScratchProcessor& processor)
    : owner(processor)
{
    stable = latest = capture();
    startTimerHz(checkHz);
}

UndoHistory::~UndoHistory()
{
    stopTimer();
}

void UndoHistory::reset()
{
    undoStack.clear();
    redoStack.clear();
    burstOpen = false;
    stable = latest = capture();
}

UndoHistory::Snapshot UndoHistory::capture() const
{
    Snapshot s;
    s.pattern = owner.getSession().getSerialisedPattern();
    s.recordPath = owner.getRecordPath();
    for (auto* parameter : owner.getParameters())
        s.params.push_back(parameter->getValue());
    return s;
}

void UndoHistory::restore(const Snapshot& s)
{
    owner.getSession().restoreSerialisedPattern(s.pattern);

    const auto& parameters = owner.getParameters();
    for (int i = 0; i < parameters.size() && i < (int) s.params.size(); ++i)
        parameters[i]->setValueNotifyingHost(s.params[(size_t) i]);

    if (s.recordPath != owner.getRecordPath())
    {
        if (s.recordPath.isEmpty())
            owner.clearRecord();
        else if (juce::File(s.recordPath).existsAsFile())
            owner.loadRecord(juce::File(s.recordPath));
    }
}

void UndoHistory::timerCallback()
{
    const auto now = capture();
    if (! (now == latest))
    {
        latest = now;
        burstOpen = true;
        lastChangeMs = juce::Time::getMillisecondCounter();
    }
    else if (burstOpen && juce::Time::getMillisecondCounter() - lastChangeMs >= quietMs)
    {
        closeBurst();
    }
}

void UndoHistory::closeBurst()
{
    if (! burstOpen)
        return;

    burstOpen = false;
    if (latest == stable)
        return;   // edited and put back: nothing to undo

    undoStack.push_back(std::move(stable));
    if (undoStack.size() > maxSteps)
        undoStack.erase(undoStack.begin());
    redoStack.clear();
    stable = latest;
}

bool UndoHistory::undo()
{
    closeBurst();   // anything just edited counts as a step first
    if (undoStack.empty())
        return false;

    redoStack.push_back(stable);
    const auto target = std::move(undoStack.back());
    undoStack.pop_back();

    restore(target);
    stable = latest = capture();
    return true;
}

bool UndoHistory::redo()
{
    closeBurst();
    if (redoStack.empty())
        return false;

    undoStack.push_back(stable);
    const auto target = std::move(redoStack.back());
    redoStack.pop_back();

    restore(target);
    stable = latest = capture();
    return true;
}
