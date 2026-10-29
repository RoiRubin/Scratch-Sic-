#include "WaveformView.h"
#include "ScratchLookAndFeel.h"

namespace
{
    constexpr int peakCount = 2048;   // detail kept for drawing; scaled to the component's width

    bool isAudioFile(const juce::String& path)
    {
        return juce::File(path).hasFileExtension("wav;aif;aiff;flac;ogg;mp3");
    }
}

void WaveformView::setRecord(std::shared_ptr<const Record> newRecord)
{
    record = std::move(newRecord);
    buildPeaks();
    repaint();
}

void WaveformView::setWindow(juce::Range<double> windowInSamples)
{
    if (windowInSamples == window)
        return;
    window = windowInSamples;
    repaint();
}

void WaveformView::setNeedle(double positionInSamples)
{
    if (juce::approximatelyEqual(positionInSamples, needle))
        return;
    needle = positionInSamples;
    repaint();
}

void WaveformView::buildPeaks()
{
    peaks.clear();
    if (record == nullptr)
        return;

    const auto& audio = record->getAudio();
    const int total = record->getNumSamples();
    const int buckets = juce::jmin(peakCount, total);
    peaks.resize((size_t) buckets);

    for (int b = 0; b < buckets; ++b)
    {
        const int start = (int) ((juce::int64) b * total / buckets);
        const int end = (int) ((juce::int64) (b + 1) * total / buckets);
        Peak peak { 0.0f, 0.0f };

        for (int ch = 0; ch < audio.getNumChannels(); ++ch)
        {
            const auto range = juce::FloatVectorOperations::findMinAndMax(audio.getReadPointer(ch, start), end - start);
            peak.low = juce::jmin(peak.low, range.getStart());
            peak.high = juce::jmax(peak.high, range.getEnd());
        }
        peaks[(size_t) b] = peak;
    }
}

float WaveformView::sampleToX(double sample) const noexcept
{
    if (record == nullptr || record->getNumSamples() == 0)
        return 0.0f;
    return (float) (sample / record->getNumSamples() * getWidth());
}

double WaveformView::xToSample(float x) const noexcept
{
    if (record == nullptr || getWidth() == 0)
        return 0.0;
    return juce::jlimit(0.0, (double) record->getNumSamples(), (double) x / getWidth() * record->getNumSamples());
}

void WaveformView::mouseDown(const juce::MouseEvent& e)
{
    dragAnchor = xToSample(e.position.x);
}

void WaveformView::mouseDrag(const juce::MouseEvent& e)
{
    if (record == nullptr || ! onWindowDragged)
        return;

    const double now = xToSample(e.position.x);
    onWindowDragged(juce::jmin(dragAnchor, now), juce::jmax(dragAnchor, now));
}

bool WaveformView::isInterestedInDragSource(const SourceDetails& details)
{
    return details.description.toString().startsWith("pattern:");
}

void WaveformView::itemDragEnter(const SourceDetails&)
{
    dragHover = true;
    repaint();
}

void WaveformView::itemDragExit(const SourceDetails&)
{
    dragHover = false;
    repaint();
}

void WaveformView::itemDropped(const SourceDetails& details)
{
    dragHover = false;
    repaint();
    if (onPatternDropped)
        onPatternDropped(details.description.toString().fromFirstOccurrenceOf("pattern:", false, false).getIntValue());
}

void WaveformView::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour(Palette::panel);
    g.fillRoundedRectangle(bounds, 6.0f);

    if (record == nullptr || peaks.empty())
    {
        g.setColour(Palette::dimText);
        g.setFont(juce::FontOptions(15.0f));
        g.drawText("Drop a sample here, or click Load sample", getLocalBounds(), juce::Justification::centred);
    }
    else
    {
        // scratch window
        const float windowLeft = sampleToX(window.getStart());
        const float windowRight = sampleToX(window.getEnd());
        g.setColour(Palette::window.withAlpha(0.12f));
        g.fillRect(windowLeft, 0.0f, windowRight - windowLeft, bounds.getHeight());
        g.setColour(Palette::window.withAlpha(0.6f));
        g.drawVerticalLine((int) windowLeft, 0.0f, bounds.getHeight());
        g.drawVerticalLine((int) windowRight, 0.0f, bounds.getHeight());

        // waveform
        const float midY = bounds.getCentreY();
        const float halfHeight = bounds.getHeight() * 0.45f;
        const int width = getWidth();
        for (int x = 0; x < width; ++x)
        {
            const auto& peak = peaks[(size_t) juce::jmin((int) peaks.size() - 1, x * (int) peaks.size() / width)];
            const bool inside = x >= windowLeft && x <= windowRight;
            g.setColour(inside ? Palette::text.withAlpha(0.85f) : Palette::wave.withAlpha(0.5f));
            g.drawVerticalLine(x, midY - peak.high * halfHeight, midY - peak.low * halfHeight + 1.0f);
        }

        // needle
        g.setColour(Palette::accent);
        g.fillRect(sampleToX(needle) - 1.0f, 0.0f, 2.0f, bounds.getHeight());

        g.setColour(Palette::dimText);
        g.setFont(juce::FontOptions(12.0f));
        g.drawText(record->getFile().getFileName(), getLocalBounds().reduced(8, 4), juce::Justification::topLeft);
        g.drawText("drag across the waveform to choose the part that gets scratched", getLocalBounds().reduced(8, 4),
                   juce::Justification::bottomLeft);
    }

    g.setColour(dragHover ? Palette::accent : Palette::outline);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, dragHover ? 2.0f : 1.0f);
}

bool WaveformView::isInterestedInFileDrag(const juce::StringArray& files)
{
    return files.size() == 1 && isAudioFile(files[0]);
}

void WaveformView::fileDragEnter(const juce::StringArray&, int, int)
{
    dragHover = true;
    repaint();
}

void WaveformView::fileDragExit(const juce::StringArray&)
{
    dragHover = false;
    repaint();
}

void WaveformView::filesDropped(const juce::StringArray& files, int, int)
{
    dragHover = false;
    repaint();
    if (onFileDropped && ! files.isEmpty())
        onFileDropped(juce::File(files[0]));
}
