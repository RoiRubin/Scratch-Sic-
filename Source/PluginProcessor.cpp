#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    const juce::Identifier recordPathProperty { "recordPath" };
    const juce::Identifier patternProperty { "pattern" };
}

ScratchProcessor::ScratchProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "STATE", Parameters::createLayout()),
      params(state),
      session(*this),
      history(*this)
{
    formats.registerBasicFormats();
}

juce::AudioProcessorEditor* ScratchProcessor::createEditor()
{
    return new ScratchEditor(*this);
}

bool ScratchProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void ScratchProcessor::prepareToPlay(double sampleRate, int maxBlockSize)
{
    currentSampleRate.store(sampleRate);

    outputGain.prepare({ sampleRate, (juce::uint32) maxBlockSize, (juce::uint32) getTotalNumOutputChannels() });
    outputGain.setRampDurationSeconds(0.02);
    outputGain.setGainDecibels(params.outputGain.load());
}

void ScratchProcessor::readHostTempo()
{
    if (auto* head = getPlayHead())
        if (const auto position = head->getPosition())
            if (const auto bpm = position->getBpm())
            {
                hostBpm.store(*bpm);
                hostTempoKnown.store(true);
            }
}

void ScratchProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    readHostTempo();
    preview.process(buffer);

    outputGain.setGainDecibels(params.outputGain.load());
    juce::dsp::AudioBlock<float> block(buffer);
    outputGain.process(juce::dsp::ProcessContextReplacing<float>(block));
}

bool ScratchProcessor::loadRecord(const juce::File& file)
{
    auto loaded = Record::loadFromFile(file, formats);
    if (loaded == nullptr)
        return false;

    {
        const juce::SpinLock::ScopedLockType lock(recordLock);
        std::swap(record, loaded);
    }
    // the previous record is released here, on the message thread

    state.state.setProperty(recordPathProperty, file.getFullPathName(), nullptr);
    return true;
}

void ScratchProcessor::clearRecord()
{
    std::shared_ptr<const Record> old;
    {
        const juce::SpinLock::ScopedLockType lock(recordLock);
        std::swap(record, old);
    }
    state.state.removeProperty(recordPathProperty, nullptr);
}

std::shared_ptr<const Record> ScratchProcessor::getRecord() const
{
    const juce::SpinLock::ScopedLockType lock(recordLock);
    return record;
}

juce::String ScratchProcessor::getRecordPath() const
{
    const auto current = getRecord();
    return current != nullptr ? current->getFile().getFullPathName() : juce::String();
}

void ScratchProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    state.state.setProperty(patternProperty, session.getSerialisedPattern(), nullptr);
    if (const auto xml = state.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void ScratchProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    const auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName(state.state.getType()))
        return;

    state.replaceState(juce::ValueTree::fromXml(*xml));
    session.restoreSerialisedPattern(state.state.getProperty(patternProperty).toString());

    const juce::File saved(state.state.getProperty(recordPathProperty).toString());
    if (saved.existsAsFile())
        loadRecord(saved);

    history.reset();   // a freshly loaded project starts with nothing to undo
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScratchProcessor();
}
