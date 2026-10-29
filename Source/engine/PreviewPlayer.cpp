#include "PreviewPlayer.h"

void PreviewPlayer::setRendered(std::shared_ptr<const RenderedScratch> newRendered)
{
    {
        const juce::SpinLock::ScopedLockType scoped(lock);
        std::swap(rendered, newRendered);
    }
    // the old sound is released here, on the message thread, never on the audio thread
}

std::shared_ptr<const RenderedScratch> PreviewPlayer::getRendered() const
{
    const juce::SpinLock::ScopedLockType scoped(lock);
    return rendered;
}

void PreviewPlayer::play() noexcept
{
    position.store(0);
    playing.store(true);
}

void PreviewPlayer::stop() noexcept
{
    playing.store(false);
}

void PreviewPlayer::process(juce::AudioBuffer<float>& output) noexcept
{
    if (! playing.load())
        return;

    const juce::SpinLock::ScopedTryLockType scoped(lock);
    if (! scoped.isLocked() || rendered == nullptr)
        return;

    const int total = rendered->getNumSamples();
    const int outChannels = output.getNumChannels();
    int pos = position.load();

    for (int n = 0; n < output.getNumSamples(); ++n)
    {
        if (pos >= total)
        {
            if (! looping.load())
            {
                playing.store(false);
                break;
            }
            pos = 0;
        }

        for (int ch = 0; ch < outChannels; ++ch)
            output.getWritePointer(ch)[n] += rendered->getSample(juce::jmin(ch, 1), pos);
        ++pos;
    }
    position.store(pos);
}
