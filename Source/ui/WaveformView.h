#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "engine/Record.h"

// Shows the loaded sample, the window the scratch moves across, and the needle.
// Drag across the waveform to choose the window. Drop an audio file on it to load it,
// or drop a scratch from the bank on it to apply that scratch to the sample.
class WaveformView : public juce::Component, public juce::FileDragAndDropTarget, public juce::DragAndDropTarget
{
public:
    std::function<void(const juce::File&)> onFileDropped;
    std::function<void(int bankRow)> onPatternDropped;
    std::function<void(double startSample, double endSample)> onWindowDragged;

    void setRecord(std::shared_ptr<const Record> newRecord);
    void setWindow(juce::Range<double> windowInSamples);
    void setNeedle(double positionInSamples);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray&, int, int) override;
    void fileDragExit(const juce::StringArray&) override;
    void filesDropped(const juce::StringArray& files, int, int) override;

    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails&) override;
    void itemDragExit(const SourceDetails&) override;
    void itemDropped(const SourceDetails& details) override;

private:
    struct Peak { float low, high; };

    void buildPeaks();
    float sampleToX(double sample) const noexcept;
    double xToSample(float x) const noexcept;

    std::shared_ptr<const Record> record;
    std::vector<Peak> peaks;
    juce::Range<double> window;
    double needle = 0.0;
    double dragAnchor = 0.0;
    bool dragHover = false;
};
