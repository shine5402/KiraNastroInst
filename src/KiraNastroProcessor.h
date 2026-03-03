// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <optional>

#include "audio/BGMPlayer.h"
#include "data/GuideBGMParser.h"
#include "data/LabelExporter.h"
#include "data/ReclistParser.h"

class KiraNastroProcessor : public juce::AudioProcessor
{
public:
    KiraNastroProcessor();
    ~KiraNastroProcessor() override;

    //==============================================================================
    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
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
    void changeProgramName(int, const juce::String &) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    //==============================================================================
    // Data loading — call from message thread
    bool loadReclist(const juce::File &reclistFile);
    bool loadGuideBGM(const juce::File &wavFile);

    // Thread-safe accessors — return copies under lock
    std::optional<ReclistData> getReclistData() const;
    std::optional<GuideBGMData> getGuideBGMData() const;
    bool isBGMLoaded() const;

    using DescExportParams = LabelExporter::Params;
    DescExportParams getDescExportParams() const;

    struct EntryInfo
    {
        juce::String name;
        juce::String comment;
        int index;
        int total;
    };
    EntryInfo getCurrentEntryInfo() const;
    EntryInfo getNextEntryInfo() const;

    // BGM control methods (for standalone)
    void startBGM();
    void stopBGM();
    void seekBGM(double seconds);
    bool isBGMPlaying() const;
    double getBGMLengthSeconds() const;

    //==============================================================================
    // Public member variables
    std::atomic<int> m_currentEntryIndex{0};
    std::atomic<double> m_projectPlayPositionSeconds{0.0};
    std::atomic<float> m_bgmLoopProgress{0.0f};
    std::atomic<int> m_totalEntries{0};

private:
    // Private member variables
    double m_currentSampleRate = 44100.0;
    int m_currentBlockSize = 512;

    juce::CriticalSection m_dataLock;
    std::optional<ReclistData> m_reclistData;
    std::optional<GuideBGMData> m_bgmData;
    BGMPlayer m_bgmPlayer;

    // Pre-computed BGM loop boundaries (in milliseconds)
    // Block = [bgmBlockStartMs, bgmBlockEndMs)
    // The first timing node defines blockStart; the node with repeatTargetNodeIndex defines blockEnd.
    double m_bgmBlockStartMs = 0.0;
    double m_bgmBlockEndMs = 0.0;

    // Recording window offsets within the BGM block (in milliseconds)
    double m_recordingStartOffsetMs = 0.0;    // ms from bgmBlockStart to isRecordingStart node
    double m_recordingWindowDurationMs = 0.0; // duration of recording window in ms

    bool m_isBGMPlayingFlag = false;       // standalone only
    bool m_wasDAWPlayingLastBlock = false; // plugin mode: detect play→stop→play

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroProcessor)
};
