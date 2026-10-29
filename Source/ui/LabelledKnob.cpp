#include "LabelledKnob.h"

LabelledKnob::LabelledKnob(juce::AudioProcessorValueTreeState& state, const juce::String& parameterId, const juce::String& name)
    : attachment(state, parameterId, slider)
{
    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    addAndMakeVisible(label);

    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 18);
    addAndMakeVisible(slider);
}

void LabelledKnob::resized()
{
    auto area = getLocalBounds();
    label.setBounds(area.removeFromTop(18));
    slider.setBounds(area);
}
