#pragma once

#include <juce_core/juce_core.h>

// Phase 2: Loads .txt reclist files (UTF-8 or Shift-JIS),
// parses entries and optional -comment.txt sidecar.
class ReclistParser
{
public:
    ReclistParser() = default;
    ~ReclistParser() = default;

    // TODO Phase 2: implement load(File), getEntries(), getComment(entry)

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReclistParser)
};
