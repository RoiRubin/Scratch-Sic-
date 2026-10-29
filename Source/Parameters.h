#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Ids of the plugin's knobs. FL Studio stores settings against these, so never rename them.
namespace ParamID
{
    inline constexpr const char* windowStart   = "windowStart";
    inline constexpr const char* windowLength  = "windowLength";
    inline constexpr const char* platterWeight = "platterWeight";
    inline constexpr const char* faderSmooth   = "faderSmooth";
    inline constexpr const char* fullSample    = "fullSample";
    inline constexpr const char* outputGain    = "outputGain";
}

// The plugin's knobs. Holds direct references to the live values so the audio thread never looks them up by name.
class Parameters
{
public:
    explicit Parameters(juce::AudioProcessorValueTreeState& state);

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    std::atomic<float>& windowStart;    // 0..1 of the whole sample
    std::atomic<float>& windowLength;   // seconds
    std::atomic<float>& platterWeight;  // 0..1, how heavy the platter feels
    std::atomic<float>& faderSmooth;    // milliseconds each fader cut takes
    std::atomic<float>& fullSample;     // 1 = play and export the whole sample, 0 = only the scratched part
    std::atomic<float>& outputGain;     // dB
};
