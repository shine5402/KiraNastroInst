// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "KiraNastroProcessor.h"

#include "KiraNastroEditor.h"

KiraNastroProcessor::KiraNastroProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

KiraNastroProcessor::~KiraNastroProcessor()
{
}

//==============================================================================
void KiraNastroProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    m_currentSampleRate = sampleRate;
    m_currentBlockSize = samplesPerBlock;
    m_bgmPlayer.prepareToPlay(sampleRate, samplesPerBlock);
}

void KiraNastroProcessor::releaseResources()
{
    m_bgmPlayer.stop();
}

bool KiraNastroProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    // Stereo output only; no audio input required (VSTi)
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void KiraNastroProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer & /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    if (!m_bgmPlayer.isLoaded())
        return;

    const bool isStandalone = (wrapperType == wrapperType_Standalone);
    double hostBlockStartSeconds = m_projectPlayPositionSeconds.load(std::memory_order_relaxed);

    // NOTE: JUCE compiles shared plugin code with JUCE_STANDALONE_APPLICATION=1
    // even for VST3/AU builds. Use wrapperType at runtime instead.
    if (isStandalone) {
        if (!m_isBGMPlayingFlag)
            return;
    }
    else {
        // In plugin mode, lock playback to the DAW transport every block.
        // This handles host seek/jump/loop correctly.
        bool dawPlaying = false;
        juce::Optional<juce::AudioPlayHead::PositionInfo> pos;
        if (auto *playHead = getPlayHead()) {
            pos = playHead->getPosition();
            if (pos.hasValue())
                dawPlaying = pos->getIsPlaying();
        }

        if (!dawPlaying) {
            if (m_wasDAWPlayingLastBlock)
                m_bgmPlayer.stop();
            m_wasDAWPlayingLastBlock = false;
            return;
        }

        if (!pos.hasValue())
            return;

        // Prefer sample-accurate host position; fall back to other timeline info.
        if (auto timeInSamples = pos->getTimeInSamples(); timeInSamples.hasValue()) {
            hostBlockStartSeconds = static_cast<double>(*timeInSamples) / m_currentSampleRate;
        }
        else if (auto timeInSeconds = pos->getTimeInSeconds(); timeInSeconds.hasValue()) {
            hostBlockStartSeconds = *timeInSeconds;
        }
        else if (auto ppq = pos->getPpqPosition(); ppq.hasValue()) {
            if (auto bpm = pos->getBpm(); bpm.hasValue() && *bpm > 0.0)
                hostBlockStartSeconds = (*ppq * 60.0) / *bpm;
        }

        hostBlockStartSeconds = std::max(hostBlockStartSeconds, 0.0);
        m_projectPlayPositionSeconds.store(hostBlockStartSeconds, std::memory_order_relaxed);

        const double blockDurationMs = m_bgmBlockEndMs - m_bgmBlockStartMs;
        const double fileSampleRate = static_cast<double>(m_bgmPlayer.getSampleRate());
        if (blockDurationMs <= 0.0 || fileSampleRate <= 0.0)
            return;

        const double hostStartMs = hostBlockStartSeconds * 1000.0;
        const int cycleIndex = static_cast<int>(std::floor(hostStartMs / blockDurationMs));

        // If a reclist is loaded, stop rendering after the last entry's cycle.
        const int total = m_totalEntries.load();
        if (total > 0 && cycleIndex >= total) {
            m_currentEntryIndex.store(total - 1);
            m_bgmPlayer.stop();
            m_wasDAWPlayingLastBlock = dawPlaying;
            return;
        }

        const int entryIndex = std::max(cycleIndex, 0);
        m_currentEntryIndex.store(entryIndex);

        double offsetWithinCycleMs = std::fmod(hostStartMs, blockDurationMs);
        if (offsetWithinCycleMs < 0.0)
            offsetWithinCycleMs += blockDurationMs;

        const double targetBgmMs = m_bgmBlockStartMs + offsetWithinCycleMs;
        const int64_t targetSample = static_cast<int64_t>((targetBgmMs / 1000.0) * fileSampleRate);
        m_bgmPlayer.seekToSample(targetSample);
        m_bgmPlayer.play();
        m_wasDAWPlayingLastBlock = dawPlaying;
    }

    // --- Simple block-loop playback ---
    // The BGM "block" is defined by [bgmBlockStartMs, bgmBlockEndMs).
    // We render audio within this range. When playback reaches blockEnd,
    // we seek back to blockStart and advance the reclist entry.

    const double blockStartMs = m_bgmBlockStartMs;
    const double blockEndMs = m_bgmBlockEndMs;

    if (blockEndMs <= blockStartMs)
        return; // invalid bounds — nothing to play

    // BGMPlayer positions are in file-sample units, so convert ms using the
    // file's sample rate, NOT the host sample rate.
    const double fileSampleRate = static_cast<double>(m_bgmPlayer.getSampleRate());
    if (fileSampleRate <= 0.0)
        return;

    const int64_t blockStartSamples = static_cast<int64_t>((blockStartMs / 1000.0) * fileSampleRate);
    const int64_t blockEndSamples = static_cast<int64_t>((blockEndMs / 1000.0) * fileSampleRate);

    const int numSamplesInBlock = buffer.getNumSamples();
    int samplesRendered = 0;

    while (samplesRendered < numSamplesInBlock) {
        const int64_t currentPos = m_bgmPlayer.getCurrentPositionSamples();

        // If we're already at or past the block end, loop back
        if (currentPos >= blockEndSamples) {
            // Advance reclist entry
            const int current = m_currentEntryIndex.load();
            const int total = m_totalEntries.load();
            if (current < total - 1) {
                m_currentEntryIndex.store(current + 1);
            }
            else {
                // End of reclist — stop playback
                m_bgmPlayer.stop();
                m_isBGMPlayingFlag = false;
                return;
            }

            m_bgmPlayer.seekToSample(blockStartSamples);
            continue; // re-evaluate position from the top
        }

        // How many host samples remain until the block boundary?
        // samplesUntilEnd is in file-sample space; convert to host samples.
        const int64_t fileSamplesUntilEnd = blockEndSamples - currentPos;
        const double playbackRatio = fileSampleRate / m_currentSampleRate;
        const int64_t hostSamplesUntilEnd =
            static_cast<int64_t>(std::ceil(static_cast<double>(fileSamplesUntilEnd) / playbackRatio));
        const int remaining = numSamplesInBlock - samplesRendered;
        const int samplesToRender = static_cast<int>(std::min(static_cast<int64_t>(remaining), hostSamplesUntilEnd));

        if (samplesToRender <= 0)
            break; // safety

        m_bgmPlayer.renderNextBlock(buffer, samplesRendered, samplesToRender);
        samplesRendered += samplesToRender;

        // Update project position
        if (isStandalone) {
            m_projectPlayPositionSeconds.store(m_projectPlayPositionSeconds.load(std::memory_order_relaxed) +
                                                   static_cast<double>(samplesToRender) / m_currentSampleRate,
                                               std::memory_order_relaxed);
        }
        else {
            m_projectPlayPositionSeconds.store(hostBlockStartSeconds +
                                                   static_cast<double>(samplesRendered) / m_currentSampleRate,
                                               std::memory_order_relaxed);
        }
    }

    // Update loop progress for UI
    const double cycleDuration = blockEndMs - blockStartMs;
    if (cycleDuration > 0.0) {
        const double curMs = (static_cast<double>(m_bgmPlayer.getCurrentPositionSamples()) / fileSampleRate) * 1000.0;
        float progress = static_cast<float>((curMs - blockStartMs) / cycleDuration);
        m_bgmLoopProgress.store(std::clamp(progress, 0.0f, 1.0f), std::memory_order_relaxed);
    }
}

//==============================================================================
juce::AudioProcessorEditor *KiraNastroProcessor::createEditor()
{
    return new KiraNastroEditor(*this);
}

//==============================================================================
void KiraNastroProcessor::getStateInformation(juce::MemoryBlock & /*destData*/)
{
    // TODO Phase 6: save reclist path, BGM path, currentEntryIndex, settings
}

void KiraNastroProcessor::setStateInformation(const void * /*data*/, int /*sizeInBytes*/)
{
    // TODO Phase 6: restore session state
}

//==============================================================================
// Data loading

bool KiraNastroProcessor::loadReclist(const juce::File &reclistFile)
{
    auto result = ReclistParser::load(reclistFile);

    juce::ScopedLock sl(m_dataLock);
    m_reclistData = result;

    if (result.has_value()) {
        m_currentEntryIndex.store(0);
        m_totalEntries.store(static_cast<int>(result->entries.size()));
        return true;
    }

    m_totalEntries.store(0);
    return false;
}

KiraNastroProcessor::BGMLoadResult KiraNastroProcessor::loadGuideBGM(const juce::File &wavFile)
{
    // Check timing file first — gives the most specific error
    auto timingFile = wavFile.withFileExtension("txt");
    if (!timingFile.existsAsFile())
        return BGMLoadResult::TimingFileMissing;

    auto timingResult = GuideBGMParser::load(wavFile);
    if (!timingResult.has_value())
        return BGMLoadResult::TimingFileInvalid;

    bool wavOk = m_bgmPlayer.loadFile(wavFile);
    if (!wavOk)
        return BGMLoadResult::WavLoadFailed;

    {
        juce::ScopedLock sl(m_dataLock);
        m_bgmData = timingResult;
    }

    const auto &t = timingResult->timing;
    m_bgmBlockStartMs           = t.bgmPlaybackStartMs;
    m_bgmBlockEndMs             = t.bgmLoopMs;
    m_recordingStartOffsetMs    = t.recordingStartMs - t.bgmPlaybackStartMs;
    m_recordingWindowDurationMs = t.recordingEndMs   - t.recordingStartMs;

    const double blockDurationMs = t.bgmLoopMs - t.bgmPlaybackStartMs;
    if (blockDurationMs > 0.0) {
        m_utteranceStartFraction.store(
            static_cast<float>((t.utteranceStartMs - t.bgmPlaybackStartMs) / blockDurationMs),
            std::memory_order_relaxed);
        m_utteranceEndFraction.store(
            static_cast<float>((t.utteranceEndMs - t.bgmPlaybackStartMs) / blockDurationMs),
            std::memory_order_relaxed);
    } else {
        m_utteranceStartFraction.store(0.333f, std::memory_order_relaxed);
        m_utteranceEndFraction.store(0.667f, std::memory_order_relaxed);
    }

    // Seek to block start
    const double fileSR = static_cast<double>(m_bgmPlayer.getSampleRate());
    const int64_t startSamples = static_cast<int64_t>((m_bgmBlockStartMs / 1000.0) * fileSR);
    m_bgmPlayer.seekToSample(startSamples);

    return BGMLoadResult::Success;
}

std::optional<ReclistData> KiraNastroProcessor::getReclistData() const
{
    juce::ScopedLock sl(m_dataLock);
    return m_reclistData;
}

KiraNastroProcessor::EntryInfo KiraNastroProcessor::getCurrentEntryInfo() const
{
    juce::ScopedLock sl(m_dataLock);
    EntryInfo info;
    info.index = m_currentEntryIndex.load();
    info.total = m_totalEntries.load();

    if (m_reclistData.has_value() && info.index >= 0 && static_cast<size_t>(info.index) < m_reclistData->entries.size())
    {
        info.name = m_reclistData->entries[static_cast<size_t>(info.index)];
        if (m_reclistData->comments.count(info.name))
            info.comment = m_reclistData->comments.at(info.name);
    }
    return info;
}

KiraNastroProcessor::EntryInfo KiraNastroProcessor::getNextEntryInfo() const
{
    juce::ScopedLock sl(m_dataLock);
    EntryInfo info;
    info.index = m_currentEntryIndex.load() + 1;
    info.total = m_totalEntries.load();

    if (m_reclistData.has_value() && info.index >= 0 && static_cast<size_t>(info.index) < m_reclistData->entries.size())
    {
        info.name = m_reclistData->entries[static_cast<size_t>(info.index)];
        if (m_reclistData->comments.count(info.name))
            info.comment = m_reclistData->comments.at(info.name);
    }
    return info;
}

std::optional<GuideBGMData> KiraNastroProcessor::getGuideBGMData() const
{
    juce::ScopedLock sl(m_dataLock);
    return m_bgmData;
}

bool KiraNastroProcessor::isBGMLoaded() const
{
    return m_bgmPlayer.isLoaded();
}

void KiraNastroProcessor::startBGM()
{
    // Play from the current position (set by loadGuideBGM or seekBGM).
    // Do NOT reset to blockStart here — that would undo any user seek.
    m_bgmPlayer.play();
    m_isBGMPlayingFlag = true;
}

void KiraNastroProcessor::stopBGM()
{
    m_bgmPlayer.stop();
    m_isBGMPlayingFlag = false;
    // If we want a true 'Stop' behavior like OREMO, we should reset.
    // But let's keep it as Pause for now to satisfy the user's resume need.
    // Actually, let's keep it as is.
}

bool KiraNastroProcessor::isBGMPlaying() const
{
    return m_isBGMPlayingFlag;
}

double KiraNastroProcessor::getBGMLengthSeconds() const
{
    if (m_bgmPlayer.isLoaded() && m_bgmBlockEndMs > m_bgmBlockStartMs) {
        return (m_bgmBlockEndMs - m_bgmBlockStartMs) / 1000.0;
    }
    return 0.0;
}

void KiraNastroProcessor::seekBGM(double seconds)
{
    if (!m_bgmPlayer.isLoaded())
        return;

    const double blockDurationMs = m_bgmBlockEndMs - m_bgmBlockStartMs;
    if (blockDurationMs <= 0.0)
        return;

    double sessionMs = std::max(seconds * 1000.0, 0.0);

    // Map session time to the loop: each block cycle corresponds to one entry.
    int entryIndex = static_cast<int>(sessionMs / blockDurationMs);
    double offsetWithinBlock = std::fmod(sessionMs, blockDurationMs);
    double targetBgmMs = m_bgmBlockStartMs + offsetWithinBlock;

    // Clamp entry index to reclist bounds
    const int total = m_totalEntries.load();
    if (total > 0) {
        m_currentEntryIndex.store(std::clamp(entryIndex, 0, total - 1));
    }

    const double fileSR = static_cast<double>(m_bgmPlayer.getSampleRate());
    const int64_t targetSamples = static_cast<int64_t>((targetBgmMs / 1000.0) * fileSR);
    m_bgmPlayer.seekToSample(targetSamples);
    m_projectPlayPositionSeconds.store(seconds, std::memory_order_relaxed);

    // Update loop progress so the timing indicator reflects the seek immediately,
    // even when paused (processBlock only runs while playing).
    const double cycleDuration = m_bgmBlockEndMs - m_bgmBlockStartMs;
    if (cycleDuration > 0.0) {
        const float progress = static_cast<float>((targetBgmMs - m_bgmBlockStartMs) / cycleDuration);
        m_bgmLoopProgress.store(std::clamp(progress, 0.0f, 1.0f), std::memory_order_relaxed);
    }
}

KiraNastroProcessor::DescExportParams KiraNastroProcessor::getDescExportParams() const
{
    DescExportParams p;
    juce::ScopedLock sl(m_dataLock);
    if (m_reclistData.has_value())
        p.entryNames = m_reclistData->entries;
    p.blockDurationSec = (m_bgmBlockEndMs - m_bgmBlockStartMs) / 1000.0;
    p.recordingStartOffsetSec = m_recordingStartOffsetMs / 1000.0;
    p.recordingWindowDurationSec = m_recordingWindowDurationMs / 1000.0;
    return p;
}

//==============================================================================
// Plugin entry point — required by JUCE
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new KiraNastroProcessor();
}
