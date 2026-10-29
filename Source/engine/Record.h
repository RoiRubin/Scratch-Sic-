#pragma once
#include <juce_audio_formats/juce_audio_formats.h>

// A loaded sample: the "vinyl" the needle reads from. It never changes after loading,
// so the audio thread and the UI can both read it without coordinating.
class Record
{
public:
    // Returns nullptr when the file can't be read as audio
    static std::shared_ptr<const Record> loadFromFile(const juce::File& file, juce::AudioFormatManager& formats);

    const juce::AudioBuffer<float>& getAudio() const noexcept { return audio; }
    int getNumSamples() const noexcept { return audio.getNumSamples(); }
    int getNumChannels() const noexcept { return audio.getNumChannels(); }
    double getSampleRate() const noexcept { return sampleRate; }
    const juce::File& getFile() const noexcept { return file; }
    juce::uint32 getId() const noexcept { return id; }   // unique per load, so changes can be spotted

    // The part of the sample the Record Position knob sweeps across, in samples
    juce::Range<double> windowFor(float startFraction, float lengthSeconds) const noexcept;

private:
    Record(juce::AudioBuffer<float> audio, double sampleRate, juce::File file);

    juce::AudioBuffer<float> audio;
    double sampleRate;
    juce::File file;
    juce::uint32 id;
};
