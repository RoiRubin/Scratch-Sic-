#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Colours shared by every part of the editor
namespace Palette
{
    inline const juce::Colour background { 0xff15171b };
    inline const juce::Colour panel      { 0xff1f2228 };
    inline const juce::Colour outline    { 0xff2e323a };
    inline const juce::Colour accent     { 0xffff6a3d };   // needle, active knob arcs
    inline const juce::Colour window     { 0xff3dd6c6 };   // the scratch window on the waveform
    inline const juce::Colour wave       { 0xff8b93a1 };
    inline const juce::Colour text       { 0xffe6e8ec };
    inline const juce::Colour dimText    { 0xff7c8390 };
}

// The editor's visual style: dark panels and flat rotary knobs
class ScratchLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ScratchLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float startAngle, float endAngle, juce::Slider& slider) override;
};
