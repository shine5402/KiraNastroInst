#pragma once

#include <juce_core/juce_core.h>

// Phase 2: Parses OREMO-format guide BGM timing description files (.txt).
// Produces a list of timing nodes used by the scheduling engine.
class GuideBGMParser
{
public:
    GuideBGMParser() = default;
    ~GuideBGMParser() = default;

    // TODO Phase 2: implement load(File), getNodes()

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuideBGMParser)
};
