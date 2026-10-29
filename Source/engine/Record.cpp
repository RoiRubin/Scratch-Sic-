#include "Record.h"

namespace
{
    constexpr double maxLengthSeconds = 600.0;   // longer files are cut, to keep memory sane

    juce::uint32 nextId()
    {
        static std::atomic<juce::uint32> counter { 0 };
        return ++counter;
    }
}

Record::Record(juce::AudioBuffer<float> audioIn, double rate, juce::File source)
    : audio(std::move(audioIn)), sampleRate(rate), file(std::move(source)), id(nextId())
{
}

std::shared_ptr<const Record> Record::loadFromFile(const juce::File& file, juce::AudioFormatManager& formats)
{
    const std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));
    if (reader == nullptr || reader->lengthInSamples <= 0 || reader->sampleRate <= 0)
        return nullptr;

    const auto length = (int) juce::jmin(reader->lengthInSamples, (juce::int64) (maxLengthSeconds * reader->sampleRate));
    const auto channels = (int) juce::jlimit(1u, 2u, reader->numChannels);

    juce::AudioBuffer<float> audio(channels, length);
    if (! reader->read(&audio, 0, length, 0, true, channels > 1))
        return nullptr;

    return std::shared_ptr<const Record>(new Record(std::move(audio), reader->sampleRate, file));
}

juce::Range<double> Record::windowFor(float startFraction, float lengthSeconds) const noexcept
{
    const double total = (double) getNumSamples();
    const double start = juce::jlimit(0.0, total, (double) startFraction * total);
    const double length = juce::jmin((double) lengthSeconds * sampleRate, total - start);
    return { start, start + length };
}
