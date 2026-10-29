#include "PluginEditor.h"

namespace
{
    constexpr int editorWidth = 940, editorHeight = 720;
    constexpr int refreshHz = 30;

    struct SnapChoice { const char* text; double step; };   // step is a fraction of the graph
    constexpr SnapChoice snapChoices[] = { { "Off", 0.0 }, { "1/4", 1.0 / 4 }, { "1/8", 1.0 / 8 }, { "1/16", 1.0 / 16 },
                                           { "1/32", 1.0 / 32 }, { "1/64", 1.0 / 64 }, { "1/128", 1.0 / 128 } };
    constexpr int defaultSnapIndex = 3;
}

ScratchEditor::ScratchEditor(ScratchProcessor& p)
    : AudioProcessorEditor(p),
      scratch(p),
      session(p.getSession()),
      graph(p.getSession()),
      bank(p.getSession()),
      exportTile(p.getSession()),
      startKnob(p.getState(), ParamID::windowStart, "Window Start"),
      lengthKnob(p.getState(), ParamID::windowLength, "Window Length"),
      weightKnob(p.getState(), ParamID::platterWeight, "Platter Weight"),
      faderKnob(p.getState(), ParamID::faderSmooth, "Fader Smooth"),
      outputKnob(p.getState(), ParamID::outputGain, "Output")
{
    setLookAndFeel(&lookAndFeel);

    waveform.onFileDropped = [this](const juce::File& f) { loadSample(f); };
    waveform.onPatternDropped = [this](int row) { applyBankEntry(row); };
    waveform.onWindowDragged = [this](double a, double b) { setWindowFromWaveform(a, b); };

    loadButton.onClick = [this] { chooseSample(); };
    playButton.onClick = [this] { togglePlay(); };
    resetButton.onClick = [this] { session.loadPattern(ScratchPattern()); };
    loopButton.onClick = [this] { scratch.getPreview().setLooping(loopButton.getToggleState()); };
    fitButton.onClick = [this] { graph.resetZoom(); };
    undoButton.onClick = [this] { scratch.getHistory().undo(); };
    redoButton.onClick = [this] { scratch.getHistory().redo(); };
    fullAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.getState(), ParamID::fullSample, fullButton);
    smoothButton.setTooltip("Make every stretch of line smooth or straight (right-click the circle on a line to set just one)");
    smoothButton.onClick = [this]
    {
        session.getPattern().getPosition().setAllSmooth(smoothButton.getToggleState());
        session.patternEdited();
        graph.refresh();
    };

    int id = 1;
    for (const auto& c : snapChoices)
        snapBox.addItem(c.text, id++);
    snapBox.onChange = [this] { graph.setSnapStep(snapChoices[snapBox.getSelectedId() - 1].step); };
    snapBox.setSelectedId(defaultSnapIndex + 1, juce::sendNotification);

    snapLabel.setFont(juce::FontOptions(12.0f));
    snapLabel.setColour(juce::Label::textColourId, Palette::dimText);
    snapLabel.setJustificationType(juce::Justification::centredRight);
    timingLabel.setJustificationType(juce::Justification::centredRight);
    timingLabel.setColour(juce::Label::textColourId, Palette::dimText);
    nameLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    nameLabel.setJustificationType(juce::Justification::centredRight);

    for (auto* c : std::initializer_list<juce::Component*> { &waveform, &graph, &bank, &exportTile, &loadButton, &playButton,
                                                             &resetButton, &fitButton, &undoButton, &redoButton,
                                                             &loopButton, &smoothButton, &fullButton,
                                                             &snapBox, &snapLabel,
                                                             &timingLabel, &nameLabel, &startKnob, &lengthKnob, &weightKnob,
                                                             &faderKnob, &outputKnob })
        addAndMakeVisible(c);

    setWantsKeyboardFocus(true);   // Ctrl+Z / Ctrl+Y work from anywhere in the window
    session.addChangeListener(this);
    syncControlsToPattern();

    setSize(editorWidth, editorHeight);
    timerCallback();
    startTimerHz(refreshHz);
}

ScratchEditor::~ScratchEditor()
{
    session.removeChangeListener(this);
    setLookAndFeel(nullptr);
}

void ScratchEditor::paint(juce::Graphics& g)
{
    g.fillAll(Palette::background);

    g.setColour(Palette::text);
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("SCRATCH(SIC)", 16, 10, 300, 28, juce::Justification::centredLeft);

    g.setColour(Palette::dimText);
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("v" SCRATCH_VERSION, 16, getHeight() - 22, 100, 18, juce::Justification::centredLeft);
}

bool ScratchEditor::keyPressed(const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    if (! mods.isCommandDown())
        return false;

    const int code = (int) juce::CharacterFunctions::toUpperCase((juce::juce_wchar) key.getKeyCode());
    if (code == 'Z')
    {
        if (mods.isShiftDown())
            scratch.getHistory().redo();
        else
            scratch.getHistory().undo();
        return true;
    }
    if (code == 'Y')
    {
        scratch.getHistory().redo();
        return true;
    }
    return false;
}

void ScratchEditor::resized()
{
    auto area = getLocalBounds().reduced(16, 10);
    area.removeFromBottom(14);   // version line

    auto header = area.removeFromTop(28);
    loadButton.setBounds(header.removeFromRight(130));
    header.removeFromRight(10);
    redoButton.setBounds(header.removeFromRight(56));
    header.removeFromRight(4);
    undoButton.setBounds(header.removeFromRight(56));
    header.removeFromRight(10);
    timingLabel.setBounds(header.removeFromRight(400));

    area.removeFromTop(8);
    bank.setBounds(area.removeFromRight(230));
    area.removeFromRight(12);

    waveform.setBounds(area.removeFromTop(110));
    area.removeFromTop(8);

    auto toolbar = area.removeFromTop(26);
    playButton.setBounds(toolbar.removeFromLeft(64));
    toolbar.removeFromLeft(4);
    loopButton.setBounds(toolbar.removeFromLeft(56));
    smoothButton.setBounds(toolbar.removeFromLeft(72));
    fullButton.setBounds(toolbar.removeFromLeft(96));
    snapLabel.setBounds(toolbar.removeFromLeft(38));
    snapBox.setBounds(toolbar.removeFromLeft(66));
    toolbar.removeFromLeft(6);
    fitButton.setBounds(toolbar.removeFromLeft(40));
    toolbar.removeFromLeft(4);
    resetButton.setBounds(toolbar.removeFromLeft(44));
    toolbar.removeFromLeft(6);
    nameLabel.setBounds(toolbar);

    area.removeFromTop(8);
    auto bottom = area.removeFromBottom(130);
    area.removeFromBottom(8);
    graph.setBounds(area);

    const int knobWidth = 92;
    for (auto* knob : { &startKnob, &lengthKnob, &weightKnob, &faderKnob, &outputKnob })
        knob->setBounds(bottom.removeFromLeft(knobWidth));
    bottom.removeFromLeft(12);
    exportTile.setBounds(bottom.reduced(0, 14));
}

void ScratchEditor::timerCallback()
{
    const auto record = scratch.getRecord();
    const auto id = record != nullptr ? record->getId() : 0u;
    if (id != shownRecordId)
    {
        shownRecordId = id;
        waveform.setRecord(record);
    }

    auto& preview = scratch.getPreview();
    const auto rendered = preview.getRendered();

    if (record != nullptr)
    {
        const auto& params = scratch.getParams();
        waveform.setWindow(record->windowFor(params.windowStart.load(), params.windowLength.load()));
    }

    double playhead = -1.0, seconds = 0.0;
    if (rendered != nullptr && rendered->getNumSamples() > 0)
    {
        seconds = rendered->lengthSeconds;
        const bool playing = preview.isPlaying();
        const int pos = juce::jlimit(0, rendered->getNumSamples() - 1, playing ? preview.getPositionSamples() : rendered->getScratchStart());
        waveform.setNeedle(rendered->getNeedle(pos));
        if (playing)
        {
            // the graph only shows the scratched part, so the playhead appears once playback reaches it
            const double inScratch = (double) (pos - rendered->getScratchStart()) / rendered->getScratchLength();
            playhead = inScratch >= 0.0 && inScratch <= 1.0 ? inScratch : -1.0;
        }
    }

    const double bpm = scratch.getHostBpm();
    graph.setPlayhead(playhead);
    graph.setTiming(seconds, bpm > 0.0 ? 60.0 / bpm : 0.0);

    // shows "on" only when every stretch is smooth, so it stays right after a single stretch is changed
    smoothButton.setToggleState(session.getPattern().getPosition().areAllSmooth(), juce::dontSendNotification);
    undoButton.setEnabled(scratch.getHistory().canUndo());
    redoButton.setEnabled(scratch.getHistory().canRedo());
    playButton.setButtonText(preview.isPlaying() ? "Stop" : "Play");
    timingLabel.setText(seconds > 0.0
                            ? juce::String(seconds, 2) + " s  =  " + juce::String(seconds * bpm / 60.0, 2) + " beats at "
                                  + juce::String(bpm, 1) + " bpm" + (scratch.hasHostTempo() ? " (FL)" : " (default)")
                            : "Load a sample to start",
                        juce::dontSendNotification);
    exportTile.repaint();
}

void ScratchEditor::changeListenerCallback(juce::ChangeBroadcaster*)
{
    syncControlsToPattern();
}

void ScratchEditor::syncControlsToPattern()
{
    const auto& pattern = session.getPattern();
    nameLabel.setText(pattern.getName(), juce::dontSendNotification);
    smoothButton.setToggleState(pattern.getPosition().areAllSmooth(), juce::dontSendNotification);
    graph.refresh();
}

void ScratchEditor::togglePlay()
{
    auto& preview = scratch.getPreview();
    if (preview.isPlaying())
        preview.stop();
    else
        preview.play();
}

void ScratchEditor::applyBankEntry(int row)
{
    const auto& entries = session.getBank().getEntries();
    if (row >= 0 && row < (int) entries.size())
        session.loadPattern(entries[(size_t) row].pattern);
}

void ScratchEditor::setParameter(juce::RangedAudioParameter& parameter, float realValue)
{
    parameter.beginChangeGesture();
    parameter.setValueNotifyingHost(parameter.convertTo0to1(realValue));
    parameter.endChangeGesture();
}

void ScratchEditor::setWindowFromWaveform(double startSample, double endSample)
{
    const auto record = scratch.getRecord();
    if (record == nullptr)
        return;

    auto& state = scratch.getState();
    const double total = record->getNumSamples();
    const float seconds = (float) ((endSample - startSample) / record->getSampleRate());
    setParameter(*state.getParameter(ParamID::windowStart), (float) (startSample / total));
    setParameter(*state.getParameter(ParamID::windowLength), seconds);
}

void ScratchEditor::chooseSample()
{
    chooser = std::make_unique<juce::FileChooser>("Load a sample to scratch", juce::File(),
                                                  scratch.getFormats().getWildcardForAllFormats());
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                         [this](const juce::FileChooser& fc)
                         {
                             if (fc.getResult().existsAsFile())
                                 loadSample(fc.getResult());
                         });
}

void ScratchEditor::loadSample(const juce::File& file)
{
    if (! scratch.loadRecord(file))
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Scratch(Sic)",
                                               "Couldn't read " + file.getFileName() + " as audio.");
}
