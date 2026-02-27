#pragma once

#include <juce_core/juce_core.h>
#include <optional>
#include <vector>

struct TimingNode
{
    double       timeMs;                // absolute time in milliseconds
    bool         isRecordingStart;
    bool         isRecordingEnd;
    bool         isSwitching;
    int          repeatTargetNodeIndex; // 0-indexed; -1 = no repeat
    juce::String comment;
};

struct GuideBGMData
{
    juce::String            name;   // basename without extension
    std::vector<TimingNode> nodes;  // ordered as in file (row 1 = index 0)
};

// Parses OREMO-format guide BGM timing description files (.txt).
// Produces a list of timing nodes used by the scheduling engine.
class GuideBGMParser
{
public:
    // Finds sibling .txt with same basename as .wav
    static std::optional<GuideBGMData> load(const juce::File& wavFile);

    // Direct parse from timing file
    static std::optional<GuideBGMData> loadFromTimingFile(const juce::File& timingFile);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuideBGMParser)
};
