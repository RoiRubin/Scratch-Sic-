#pragma once
#include "PluginProcessor.h"
#include "ui/ScratchLookAndFeel.h"
#include "ui/LabelledKnob.h"
#include "ui/WaveformView.h"
#include "ui/BankPanel.h"
#include "ui/ExportTile.h"
#include "ui/graph/GraphEditor.h"

// The plugin window: sample and waveform on top, the scratching editor in the middle,
// the knobs and the drag-out tile below, and the scratch bank on the right.
class ScratchEditor : public juce::AudioProcessorEditor,
                      public juce::DragAndDropContainer,
                      private juce::Timer,
                      private juce::ChangeListener
{
public:
    explicit ScratchEditor(ScratchProcessor& processor);
    ~ScratchEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override { grabKeyboardFocus(); }
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster*) override;

    void chooseSample();
    void loadSample(const juce::File& file);
    void applyBankEntry(int row);
    void setWindowFromWaveform(double startSample, double endSample);
    void syncControlsToPattern();
    void togglePlay();
    static void setParameter(juce::RangedAudioParameter& parameter, float realValue);

    ScratchProcessor& scratch;
    ScratchSession& session;
    ScratchLookAndFeel lookAndFeel;

    WaveformView waveform;
    GraphEditor graph;
    BankPanel bank;
    ExportTile exportTile;

    juce::TextButton loadButton { "Load sample" }, playButton { "Play" }, resetButton { "New" }, fitButton { "Fit" };
    juce::TextButton undoButton { "Undo" }, redoButton { "Redo" };
    juce::ToggleButton loopButton { "Loop" }, smoothButton { "Smooth" }, fullButton { "Full sample" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fullAttachment;
    juce::ComboBox snapBox;
    juce::Label snapLabel { {}, "Snap" }, timingLabel, nameLabel;
    LabelledKnob startKnob, lengthKnob, weightKnob, faderKnob, outputKnob;
    std::unique_ptr<juce::FileChooser> chooser;

    juce::uint32 shownRecordId = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScratchEditor)
};
