// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>

#include <map>
#include <optional>
#include <vector>

struct ReclistData
{
    juce::String name;                             // basename without extension
    std::vector<juce::String> entries;             // parsed entry list, in order
    std::map<juce::String, juce::String> comments; // entry → comment string (may be empty)
};

// Loads .txt reclist files (UTF-8 or Shift-JIS),
// parses entries and optional -comment.txt sidecar.
class ReclistParser
{
public:
    static std::optional<ReclistData> load(const juce::File &reclistFile);

    // Load from in-memory data (e.g. embedded binary resources).
    // name: display name for the reclist (no extension needed).
    // commentData/commentSize: optional comment buffer (may be nullptr/0).
    static std::optional<ReclistData> loadFromMemory(const void *reclistData, size_t reclistSize,
                                                     const juce::String &name,
                                                     const void *commentData = nullptr,
                                                     size_t commentSize = 0);

    static juce::String decodeBuffer(const void *data, size_t size);

private:
    // Returns entry → comment map; empty if file absent or unreadable
    static std::map<juce::String, juce::String> parseCommentFile(const juce::File &commentFile);
    static std::map<juce::String, juce::String> parseCommentText(const juce::String &text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReclistParser)
};
