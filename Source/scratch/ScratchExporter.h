#pragma once
#include "RenderedScratch.h"
#include <juce_core/juce_core.h>

// Saves a rendered scratch as a 24-bit WAV the user can drag into FL Studio
namespace ScratchExporter
{
    juce::File getBounceFolder();

    // Returns the written file, or a non-existent file if writing failed
    juce::File writeWav(const RenderedScratch& scratch, float gainDb);
}
