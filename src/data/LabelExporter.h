#pragma once

#include <juce_core/juce_core.h>

#include "PluginProcessor.h"

// Generates a KiraWavTar-compatible .kirawavtar-desc.json from BGM timing data.
// Entry timing is computed deterministically from the BGM block structure —
// no per-session tracking is needed.
class LabelExporter
{
public:
    // Returns true on success. On failure, errorMessage is set.
    static bool exportToFile(const juce::File &destFile, const KiraNastroProcessor::DescExportParams &params,
                             juce::String &errorMessage);

private:
    // Convert seconds to HH:MM:SS.fff timecode (KiraWavTar format)
    static juce::String toTimecode(double seconds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabelExporter)
};
