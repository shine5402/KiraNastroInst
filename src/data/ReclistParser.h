#pragma once

#include <juce_core/juce_core.h>
#include <map>
#include <optional>
#include <vector>

struct ReclistData
{
    juce::String              name;     // basename without extension
    std::vector<juce::String> entries;  // parsed entry list, in order
    std::map<juce::String, juce::String> comments; // entry → comment string (may be empty)
};

// Loads .txt reclist files (UTF-8 or Shift-JIS),
// parses entries and optional -comment.txt sidecar.
class ReclistParser
{
public:
    static std::optional<ReclistData> load(const juce::File& reclistFile);

private:
    // Returns entry → comment map; empty if file absent or unreadable
    static std::map<juce::String, juce::String>
        parseCommentFile(const juce::File& commentFile);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReclistParser)
};
