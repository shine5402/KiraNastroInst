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

    bool loadFile(const juce::File& wavFile);
    void unload();

    bool isLoaded()        const noexcept { return loaded; }
    int  getSampleRate()   const noexcept { return loadedSampleRate; }
    int  getNumChannels()  const noexcept;
    int  getTotalSamples() const noexcept;

    // Phase 3 stubs (commented out — add implementations in Phase 3):
    // void prepareToPlay(double hostSampleRate, int blockSize);
    // void renderNextBlock(juce::AudioBuffer<float>&, int start, int num);
    // void seekToSample(int64_t pos);
    // void play(); void stop();
    // bool isPlaying() const noexcept;
    // int64_t getCurrentPositionSamples() const noexcept;

    // Thread safety note for Phase 3:
    // audioBuffer is written once on the message thread (loadFile), then read-only
    // from processBlock. The position counter added in Phase 3 must be
    // std::atomic<int64_t> — never lock inside renderNextBlock.

private:
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> audioBuffer;
    int  loadedSampleRate = 0;
    bool loaded           = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BGMPlayer)
};
