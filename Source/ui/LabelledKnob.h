#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// A rotary knob with its name above and its value below, wired to one plugin parameter
class LabelledKnob : public juce::Component
{
public:
    LabelledKnob(juce::AudioProcessorValueTreeState& state, const juce::String& parameterId, const juce::String& name);

    void resized() override;

private:
    juce::Label label;
    juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};
