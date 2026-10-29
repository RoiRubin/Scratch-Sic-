#include "Parameters.h"

namespace
{
    std::atomic<float>& valueOf(juce::AudioProcessorValueTreeState& state, const char* id)
    {
        return *state.getRawParameterValue(id);
    }

    juce::String asPercent(float value, int) { return juce::String(value * 100.0f, 1) + " %"; }
    juce::String asSeconds(float value, int) { return juce::String(value, 2) + " s"; }
    juce::String asDecibels(float value, int) { return juce::String(value, 1) + " dB"; }
    juce::String asMilliseconds(float value, int) { return juce::String(value, 1) + " ms"; }
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& state)
    : windowStart(valueOf(state, ParamID::windowStart)),
      windowLength(valueOf(state, ParamID::windowLength)),
      platterWeight(valueOf(state, ParamID::platterWeight)),
      faderSmooth(valueOf(state, ParamID::faderSmooth)),
      fullSample(valueOf(state, ParamID::fullSample)),
      outputGain(valueOf(state, ParamID::outputGain))
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createLayout()
{
    using Float = juce::AudioParameterFloat;
    using Attributes = juce::AudioParameterFloatAttributes;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<Float>(juce::ParameterID { ParamID::windowStart, 1 }, "Window Start",
                                       juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f,
                                       Attributes().withStringFromValueFunction(asPercent)));

    // skewed so the short lengths used for tight scratches get most of the knob's travel
    layout.add(std::make_unique<Float>(juce::ParameterID { ParamID::windowLength, 1 }, "Window Length",
                                       juce::NormalisableRange<float>(0.02f, 10.0f, 0.0f, 0.4f), 1.0f,
                                       Attributes().withStringFromValueFunction(asSeconds)));

    layout.add(std::make_unique<Float>(juce::ParameterID { ParamID::platterWeight, 1 }, "Platter Weight",
                                       juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f,
                                       Attributes().withStringFromValueFunction(asPercent)));

    // how long each fader cut takes: short is snappy and hard, long is soft and rounded
    layout.add(std::make_unique<Float>(juce::ParameterID { ParamID::faderSmooth, 1 }, "Fader Smooth",
                                       juce::NormalisableRange<float>(0.5f, 20.0f, 0.0f, 0.5f), 4.0f,
                                       Attributes().withStringFromValueFunction(asMilliseconds)));

    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { ParamID::fullSample, 1 }, "Full Sample", true));

    layout.add(std::make_unique<Float>(juce::ParameterID { ParamID::outputGain, 1 }, "Output",
                                       juce::NormalisableRange<float>(-24.0f, 6.0f), 0.0f,
                                       Attributes().withStringFromValueFunction(asDecibels)));

    return layout;
}
