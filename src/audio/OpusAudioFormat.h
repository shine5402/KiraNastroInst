// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_audio_formats/juce_audio_formats.h>

// Read-only AudioFormat subclass for Opus (.opus) files via libopusfile.
// Always decodes to 48 kHz float PCM (Opus fixed rate).
class OpusAudioFormat : public juce::AudioFormat
{
public:
    OpusAudioFormat();
    ~OpusAudioFormat() override = default;

    juce::Array<int> getPossibleSampleRates() override;
    juce::Array<int> getPossibleBitDepths() override;
    bool canDoStereo() override;
    bool canDoMono() override;
    bool isCompressed() override;

    juce::AudioFormatReader* createReaderFor(juce::InputStream* sourceStream,
                                             bool deleteStreamIfOpeningFails) override;

    // Write support not implemented — Opus encoding is out of scope.
    std::unique_ptr<juce::AudioFormatWriter> createWriterFor(
        std::unique_ptr<juce::OutputStream>& streamToWriteTo,
        const juce::AudioFormatWriterOptions& options) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpusAudioFormat)
};
