// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>

#include <optional>

struct GuideBGMTiming
{
    double bgmPlaybackStartMs; // Row 1 — block start
    double recordingStartMs;   // Row 2 — isRecordingStart=1
    double utteranceStartMs;   // Row 3 — singer cue (begin)
    double utteranceEndMs;     // Row 4 — singer cue (end)
    double recordingEndMs;     // Row 5 — isRecordingEnd=1
    double bgmLoopMs;          // Row 6 — isSwitching=1, block end
};

struct GuideBGMData
{
    juce::String name;    // basename without extension
    GuideBGMTiming timing;
};

// Parses OREMO-format guide BGM timing description files (.txt).
// Requires exactly 6 data rows matching the fixed OREMO standard structure.
class GuideBGMParser
{
public:
    // Finds sibling .txt with same basename as .wav
    static std::optional<GuideBGMData> load(const juce::File &wavFile);

    // Direct parse from timing file
    static std::optional<GuideBGMData> loadFromTimingFile(const juce::File &timingFile);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuideBGMParser)
};
