#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

// Phase 3: BGM playback engine.
// Loads a WAV file into memory and renders it into the audio buffer,
// tracking position against guide BGM timing nodes.
class BGMPlayer
{
public:
    BGMPlayer() = default;
    ~BGMPlayer() = default;

    // TODO Phase 3: implement load, play, stop, seek, loopTo, render

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BGMPlayer)
};
