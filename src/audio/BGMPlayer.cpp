// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BGMPlayer.h"

BGMPlayer::BGMPlayer()
{
    m_formatManager.registerBasicFormats();
}

bool BGMPlayer::loadFile(const juce::File &wavFile)
{
    unload();

    std::unique_ptr<juce::AudioFormatReader> reader(m_formatManager.createReaderFor(wavFile));

    if (reader == nullptr)
        return false;

    // Guard against unreasonably large files (> 5 minutes at 48 kHz)
    const int64_t maxSamples = static_cast<int64_t>(48000) * 60 * 5;
    if (reader->lengthInSamples > maxSamples)
        return false;

    auto numChannels = static_cast<int>(reader->numChannels);
    auto numSamples = static_cast<int>(reader->lengthInSamples);

    m_audioBuffer.setSize(numChannels, numSamples);
    reader->read(&m_audioBuffer, 0, numSamples, 0, true, true);

    m_loadedSampleRate = static_cast<int>(reader->sampleRate);
    m_loaded = true;

    // Recompute playback ratio in case prepareToPlay was called before loading
    if (m_hostSampleRate > 0.0 && m_loadedSampleRate > 0)
        m_playbackRatio = static_cast<double>(m_loadedSampleRate) / m_hostSampleRate;

    return true;
}

void BGMPlayer::unload()
{
    m_audioBuffer.setSize(0, 0);
    m_loadedSampleRate = 0;
    m_loaded = false;
}

int BGMPlayer::getNumChannels() const noexcept
{
    return m_audioBuffer.getNumChannels();
}

int BGMPlayer::getTotalSamples() const noexcept
{
    return m_audioBuffer.getNumSamples();
}

void BGMPlayer::prepareToPlay(double sampleRate, int /*blockSize*/)
{
    m_hostSampleRate = sampleRate;

    if (m_loadedSampleRate > 0)
        m_playbackRatio = static_cast<double>(m_loadedSampleRate) / sampleRate;
    else
        m_playbackRatio = 1.0;
}

void BGMPlayer::renderNextBlock(juce::AudioBuffer<float> &buffer, int startSample, int numSamples)
{
    if (!m_loaded || !m_isPlayingFlag.load(std::memory_order_relaxed))
        return;

    auto currentPos = m_currentPositionSamples.load(std::memory_order_relaxed);
    const auto totalSamples = static_cast<int64_t>(m_audioBuffer.getNumSamples());

    for (int sample = 0; sample < numSamples; ++sample) {
        const auto bufferPos = startSample + sample;

        // Produce audio only if within buffer
        if (currentPos < totalSamples) {
            for (int ch = 0; ch < m_audioBuffer.getNumChannels(); ++ch) {
                if (ch < buffer.getNumChannels()) {
                    buffer.addSample(ch, bufferPos, m_audioBuffer.getSample(ch, static_cast<int>(currentPos)));
                }
            }
        }

        // ALWAYS advance position with fractional accuracy, even past EOF
        m_positionFraction += m_playbackRatio;
        currentPos += static_cast<int64_t>(m_positionFraction);
        m_positionFraction -= static_cast<int64_t>(m_positionFraction);
    }

    m_currentPositionSamples.store(currentPos, std::memory_order_relaxed);
}

void BGMPlayer::seekToSample(int64_t pos)
{
    const auto totalSamples = static_cast<int64_t>(m_audioBuffer.getNumSamples());
    m_currentPositionSamples.store(std::clamp(pos, int64_t(0), totalSamples), std::memory_order_relaxed);
    // Reset position fraction when seeking
    m_positionFraction = 0.0;
}

void BGMPlayer::play()
{
    m_isPlayingFlag.store(true, std::memory_order_relaxed);
}

void BGMPlayer::stop()
{
    m_isPlayingFlag.store(false, std::memory_order_relaxed);
}

bool BGMPlayer::isPlaying() const noexcept
{
    return m_isPlayingFlag.load(std::memory_order_relaxed);
}

int64_t BGMPlayer::getCurrentPositionSamples() const noexcept
{
    return m_currentPositionSamples.load(std::memory_order_relaxed);
}
