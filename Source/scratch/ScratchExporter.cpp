#include "ScratchExporter.h"
#include <juce_audio_formats/juce_audio_formats.h>

juce::File ScratchExporter::getBounceFolder()
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Scratch(Sic) Bounces");
}

juce::File ScratchExporter::writeWav(const RenderedScratch& scratch, float gainDb)
{
    const auto folder = getBounceFolder();
    if (! folder.createDirectory().wasOk())
        return {};

    const double totalSeconds = scratch.getNumSamples() / scratch.sampleRate;
    const auto stem = juce::File::createLegalFileName(scratch.patternName + "_" + juce::String(juce::roundToInt(totalSeconds * 1000.0))
                                                      + "ms" + (scratch.hasSurround() ? "_full" : ""));
    auto file = folder.getChildFile(stem + ".wav");
    if (file.existsAsFile() && ! file.deleteFile())
        file = folder.getNonexistentChildFile(stem, ".wav", false);   // the old one is in use, e.g. still being dragged

    auto stream = std::make_unique<juce::FileOutputStream>(file);
    if (! stream->openedOk())
        return {};

    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> writer(wav.createWriterFor(stream.get(), scratch.sampleRate, 2, 24, {}, 0));
    if (writer == nullptr)
        return {};
    stream.release();   // the writer owns the stream now

    auto audio = scratch.toBuffer();
    audio.applyGain(juce::Decibels::decibelsToGain(gainDb));
    writer->writeFromAudioSampleBuffer(audio, 0, audio.getNumSamples());
    writer.reset();     // closes the file
    return file;
}
