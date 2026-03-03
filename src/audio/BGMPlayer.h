// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

// BGM playback engine.
// Loads a WAV file into memory and exposes it for rendering in processBlock.
// Phase 3 will add the actual playback/seeking API.
class BGMPlayer
{
public:
    BGMPlayer();
    ~BGMPlayer() = default;

    bool loadFile(const juce::File &wavFile);
    void unload();

    bool isLoaded() const noexcept { return m_loaded; }
    int getSampleRate() const noexcept { return m_loadedSampleRate; }
    int getNumChannels() const noexcept;
    int getTotalSamples() const noexcept;

    // Phase 3 implementation:
    void prepareToPlay(double hostSampleRate, int blockSize);
    void renderNextBlock(juce::AudioBuffer<float> &, int start, int num);
    void seekToSample(int64_t pos);
    void play();
    void stop();
    bool isPlaying() const noexcept;
    int64_t getCurrentPositionSamples() const noexcept;

private:
    juce::AudioFormatManager m_formatManager;
    juce::AudioBuffer<float> m_audioBuffer;
    int m_loadedSampleRate = 0;
    bool m_loaded = false;

    // Playback state
    std::atomic<int64_t> m_currentPositionSamples{0};
    std::atomic<bool> m_isPlayingFlag{false};
    double m_hostSampleRate = 44100.0;
    double m_playbackRatio = 1.0;
    double m_positionFraction = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BGMPlayer)
};
