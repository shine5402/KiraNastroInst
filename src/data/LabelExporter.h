// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>

#include <vector>

// Generates a KiraWavTar-compatible .kirawavtar-desc.json from BGM timing data.
// Entry timing is computed deterministically from the BGM block structure —
// no per-session tracking is needed.
class LabelExporter
{
public:
    struct Params
    {
        std::vector<juce::String> entryNames;
        double blockDurationSec = 0.0;
        double recordingStartOffsetSec = 0.0;
        double recordingWindowDurationSec = 0.0;
        bool isValid() const
        {
            return !entryNames.empty() && blockDurationSec > 0.0 && recordingWindowDurationSec > 0.0;
        }
    };

    // Returns true on success. On failure, errorMessage is set.
    static bool exportToFile(const juce::File &destFile, const Params &params, juce::String &errorMessage);

private:
    // Convert seconds to HH:MM:SS.fff timecode (KiraWavTar format)
    static juce::String toTimecode(double seconds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabelExporter)
};
