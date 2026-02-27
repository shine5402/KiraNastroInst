#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <optional>

#include "data/ReclistParser.h"
#include "data/GuideBGMParser.h"
#include "audio/BGMPlayer.h"

class KiraNastroProcessor : public juce::AudioProcessor
{
public:
    KiraNastroProcessor();
    ~KiraNastroProcessor() override;

    //==============================================================================
    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Data loading — call from message thread
    bool loadReclist(const juce::File& reclistFile);
    bool loadGuideBGM(const juce::File& wavFile);

    // Thread-safe accessors — return copies under lock
    std::optional<ReclistData>  getReclistData()  const;
    std::optional<GuideBGMData> getGuideBGMData() const;
    bool isBGMLoaded() const;

    //==============================================================================
    // Public atomic state — safe to read from UI thread
    std::atomic<int>    currentEntryIndex    { 0 };
    std::atomic<double> bgmPlayPositionSamples { 0.0 };
    std::atomic<int>    totalEntries         { 0 };

private:
    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;

    juce::CriticalSection       dataLock;
    std::optional<ReclistData>  reclistData;
    std::optional<GuideBGMData> bgmData;
    BGMPlayer                   bgmPlayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroProcessor)
};
