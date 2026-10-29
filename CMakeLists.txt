cmake_minimum_required(VERSION 3.22)

# Bump this with every update: it sets the VST3 version FL Studio reports and the one shown in the editor
project(ScratchSic VERSION 0.6.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(ScratchSic
    COMPANY_NAME "Me"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Bbxd
    PLUGIN_CODE Scsc
    FORMATS VST3
    COPY_PLUGIN_AFTER_BUILD TRUE
    PRODUCT_NAME "Scratch(Sic)")

target_sources(ScratchSic PRIVATE
    Source/PluginProcessor.h      Source/PluginProcessor.cpp
    Source/PluginEditor.h         Source/PluginEditor.cpp
    Source/Parameters.h           Source/Parameters.cpp
    Source/UndoHistory.h          Source/UndoHistory.cpp

    Source/engine/Record.h        Source/engine/Record.cpp
    Source/engine/Hermite.h
    Source/engine/Resampler.h
    Source/engine/Platter.h       Source/engine/Platter.cpp
    Source/engine/MotionGate.h    Source/engine/MotionGate.cpp
    Source/engine/Crossfader.h    Source/engine/Crossfader.cpp
    Source/engine/Turntable.h     Source/engine/Turntable.cpp
    Source/engine/PreviewPlayer.h Source/engine/PreviewPlayer.cpp

    Source/scratch/PositionLane.h     Source/scratch/PositionLane.cpp
    Source/scratch/GateLane.h         Source/scratch/GateLane.cpp
    Source/scratch/ScratchPattern.h   Source/scratch/ScratchPattern.cpp
    Source/scratch/FactoryPatterns.h  Source/scratch/FactoryPatterns.cpp
    Source/scratch/PatternBank.h      Source/scratch/PatternBank.cpp
    Source/scratch/RenderedScratch.h  Source/scratch/RenderedScratch.cpp
    Source/scratch/ScratchRenderer.h  Source/scratch/ScratchRenderer.cpp
    Source/scratch/ScratchExporter.h  Source/scratch/ScratchExporter.cpp
    Source/scratch/ScratchSession.h   Source/scratch/ScratchSession.cpp

    Source/ui/ScratchLookAndFeel.h  Source/ui/ScratchLookAndFeel.cpp
    Source/ui/LabelledKnob.h        Source/ui/LabelledKnob.cpp
    Source/ui/WaveformView.h        Source/ui/WaveformView.cpp
    Source/ui/BankPanel.h           Source/ui/BankPanel.cpp
    Source/ui/ExportTile.h          Source/ui/ExportTile.cpp
    Source/ui/graph/GridMapper.h
    Source/ui/graph/PositionLaneView.h  Source/ui/graph/PositionLaneView.cpp
    Source/ui/graph/GateLaneView.h      Source/ui/graph/GateLaneView.cpp
    Source/ui/graph/GraphEditor.h       Source/ui/graph/GraphEditor.cpp)

target_include_directories(ScratchSic PRIVATE Source)

target_compile_definitions(ScratchSic PUBLIC
    SCRATCH_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(ScratchSic
    PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
