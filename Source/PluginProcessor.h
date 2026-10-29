#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Parameters.h"
#include "engine/Record.h"
#include "engine/PreviewPlayer.h"
#include "scratch/ScratchSession.h"
#include "UndoHistory.h"

// The Channel Rack instrument. Owns the knobs, the loaded record, the scratch being edited and the
// preview player, and saves everything with the FL project.
class ScratchProcessor : public juce::AudioProcessor
{
public:
    ScratchProcessor();
    ~ScratchProcessor() override = default;

    //---- juce::AudioProcessor
    const juce::String getName() const override { return "Scratch(Sic)"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void prepareToPlay(double sampleRate, int maxBlockSize) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //---- used by the editor and the session (message thread)
    bool loadRecord(const juce::File& file);
    void clearRecord();
    std::shared_ptr<const Record> getRecord() const;
    juce::String getRecordPath() const;
    juce::AudioProcessorValueTreeState& getState() noexcept { return state; }
    juce::AudioFormatManager& getFormats() noexcept { return formats; }
    const Parameters& getParams() const noexcept { return params; }
    PreviewPlayer& getPreview() noexcept { return preview; }
    ScratchSession& getSession() noexcept { return session; }
    UndoHistory& getHistory() noexcept { return history; }

    double getHostBpm() const noexcept { return hostBpm.load(); }
    bool hasHostTempo() const noexcept { return hostTempoKnown.load(); }
    double getCurrentSampleRate() const noexcept { return currentSampleRate.load(); }

private:
    void readHostTempo();

    juce::AudioFormatManager formats;
    juce::AudioProcessorValueTreeState state;
    Parameters params;

    PreviewPlayer preview;
    juce::dsp::Gain<float> outputGain;

    std::atomic<double> hostBpm { 120.0 };
    std::atomic<bool> hostTempoKnown { false };
    std::atomic<double> currentSampleRate { 0.0 };

    // Swapped by the UI when a new sample loads; the audio thread never touches it
    std::shared_ptr<const Record> record;
    mutable juce::SpinLock recordLock;

    ScratchSession session;
    UndoHistory history;      // last, so it stops before anything it watches goes away

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScratchProcessor)
};
