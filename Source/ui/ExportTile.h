#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "scratch/ScratchSession.h"

// Drag this out of the plugin and drop it on FL Studio's playlist (or Channel Rack / Edison)
// to bring in the scratched sound as a WAV.
class ExportTile : public juce::Component
{
public:
    explicit ExportTile(ScratchSession& sessionToExport) : session(sessionToExport) { setMouseCursor(juce::MouseCursor::DraggingHandCursor); }

    void paint(juce::Graphics& g) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent&) override { dragging = false; }

private:
    ScratchSession& session;
    bool dragging = false;
};
