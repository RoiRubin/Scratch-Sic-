#include "ScratchLookAndFeel.h"

ScratchLookAndFeel::ScratchLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, Palette::background);
    setColour(juce::Label::textColourId, Palette::text);
    setColour(juce::Slider::textBoxTextColourId, Palette::text);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::TextButton::buttonColourId, Palette::panel);
    setColour(juce::TextButton::textColourOffId, Palette::text);
    setColour(juce::TextButton::buttonOnColourId, Palette::accent);
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    setColour(juce::ComboBox::outlineColourId, Palette::outline);
    setColour(juce::ComboBox::backgroundColourId, Palette::panel);
    setColour(juce::ComboBox::textColourId, Palette::text);
    setColour(juce::ComboBox::arrowColourId, Palette::text);
    setColour(juce::PopupMenu::backgroundColourId, Palette::panel);
    setColour(juce::PopupMenu::textColourId, Palette::text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Palette::accent);
    setColour(juce::ScrollBar::thumbColourId, Palette::dimText.withAlpha(0.7f));
    setColour(juce::ScrollBar::backgroundColourId, Palette::panel);
    setColour(juce::ToggleButton::textColourId, Palette::text);
    setColour(juce::ToggleButton::tickColourId, Palette::accent);
    setColour(juce::ToggleButton::tickDisabledColourId, Palette::dimText);
}

void ScratchLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                          float startAngle, float endAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(6.0f);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float arcRadius = radius - 3.0f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    juce::Path track;
    track.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(Palette::outline);
    g.strokePath(track, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
    g.setColour(Palette::accent);
    g.strokePath(value, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float knobRadius = arcRadius - 7.0f;
    g.setColour(Palette::panel);
    g.fillEllipse(juce::Rectangle<float>(knobRadius * 2.0f, knobRadius * 2.0f).withCentre(centre));

    const auto tip = centre.getPointOnCircumference(knobRadius - 4.0f, angle);
    g.setColour(Palette::text);
    g.drawLine({ centre.getPointOnCircumference(knobRadius * 0.35f, angle), tip }, 2.0f);
}
