// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>

#include <vector>

// Registry for built-in reclists and BGM files embedded as binary data.
namespace BuiltinResources
{

struct ReclistInfo
{
    int id;
    const char *displayName;   // e.g. "JP - 巽式日本語 VCV"
    const char *attribution;   // e.g. "185 entries; 7-mora, 巽、Haru.jpg"
    const void *reclistData;
    size_t reclistSize;
    const void *commentData;   // may be nullptr
    size_t commentSize;
};

struct BGMEntry
{
    int tempo;
    const char *key;       // internal key: "C", "Cs" (displayed as "C#"), etc.
    const char *displayKey; // "C#" for UI
    const void *audioData;
    size_t audioSize;
    const void *timingData;
    size_t timingSize;
};

const std::vector<ReclistInfo> &getReclists();

const std::vector<int> &getTempos();          // {100, 120, 140}
const std::vector<juce::String> &getKeys();   // {"C","C#","D",...,"B"}

// Returns nullptr if the combination is not found
const BGMEntry *findBGM(int tempo, const juce::String &key);

} // namespace BuiltinResources
