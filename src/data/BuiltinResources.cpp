// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BuiltinResources.h"

#include "BinaryDataBGM.h"
#include "BinaryDataReclists.h"

namespace BuiltinResources
{

const std::vector<ReclistInfo> &getReclists()
{
    static const std::vector<ReclistInfo> reclists = {
        {
            0,
            u8"JP - 巽式日本語 VCV",
            u8"185 entries; 7-mora, 巽、Haru.jpg (romaji comment)",
            BinaryDataReclists::vcv_7mora_txt,
            BinaryDataReclists::vcv_7mora_txtSize,
            BinaryDataReclists::vcv_7mora_comment_txt,
            BinaryDataReclists::vcv_7mora_comment_txtSize,
        },
        {
            1,
            u8"JP - 巽式日本語 VCV (New7mora)",
            u8"195 entries; 7-mora, 巽、Haru.jpg (romaji comment)",
            BinaryDataReclists::vcv_new7mora_txt,
            BinaryDataReclists::vcv_new7mora_txtSize,
            BinaryDataReclists::vcv_new7mora_comment_txt,
            BinaryDataReclists::vcv_new7mora_comment_txtSize,
        },
        {
            2,
            u8"JP - 巽式日本語 CVVC",
            u8"82 entries; 6-mora, 巽",
            BinaryDataReclists::cvvc_6mora_txt,
            BinaryDataReclists::cvvc_6mora_txtSize,
            nullptr,
            0,
        },
        {
            3,
            u8"JP - 巽式日本語 CVVC (lite)",
            u8"46 entries; 6-mora, 巽",
            BinaryDataReclists::cvvc_6mora_lite_txt,
            BinaryDataReclists::cvvc_6mora_lite_txtSize,
            nullptr,
            0,
        },
        {
            4,
            u8"ZH - Hr.J 中文CVVC",
            u8"291 entries; 7-mora, Haru.jpg、sder.colin",
            BinaryDataReclists::zh_cvvc_7_txt,
            BinaryDataReclists::zh_cvvc_7_txtSize,
            nullptr,
            0,
        },
        {
            5,
            u8"ZH - Hr.J 中文CVVC (lite)",
            u8"216 entries; 7-mora, Haru.jpg、sder.colin",
            BinaryDataReclists::zh_cvvc_7_lite_txt,
            BinaryDataReclists::zh_cvvc_7_lite_txtSize,
            nullptr,
            0,
        },
    };
    return reclists;
}

const std::vector<int> &getTempos()
{
    static const std::vector<int> tempos = {100, 120, 140};
    return tempos;
}

const std::vector<juce::String> &getKeys()
{
    static const std::vector<juce::String> keys = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return keys;
}

const BGMEntry *findBGM(int tempo, const juce::String &key)
{
    struct Entry
    {
        int tempo;
        juce::String key; // display key e.g. "C#"
        BGMEntry entry;
    };

    static const std::vector<Entry> table = [] {
        struct Row
        {
            int tempo;
            const char *displayKey;
            const char *internalKey;
            const void *audioData;
            size_t audioSize;
            const void *timingData;
            size_t timingSize;
        };

        // clang-format off
        static const Row rows[] = {
            {100, "C",  "C",  BinaryDataBGM::Jazz100C_opus,  BinaryDataBGM::Jazz100C_opusSize,  BinaryDataBGM::Jazz100C_txt,  BinaryDataBGM::Jazz100C_txtSize},
            {100, "C#", "Cs", BinaryDataBGM::Jazz100Cs_opus, BinaryDataBGM::Jazz100Cs_opusSize, BinaryDataBGM::Jazz100Cs_txt, BinaryDataBGM::Jazz100Cs_txtSize},
            {100, "D",  "D",  BinaryDataBGM::Jazz100D_opus,  BinaryDataBGM::Jazz100D_opusSize,  BinaryDataBGM::Jazz100D_txt,  BinaryDataBGM::Jazz100D_txtSize},
            {100, "D#", "Ds", BinaryDataBGM::Jazz100Ds_opus, BinaryDataBGM::Jazz100Ds_opusSize, BinaryDataBGM::Jazz100Ds_txt, BinaryDataBGM::Jazz100Ds_txtSize},
            {100, "E",  "E",  BinaryDataBGM::Jazz100E_opus,  BinaryDataBGM::Jazz100E_opusSize,  BinaryDataBGM::Jazz100E_txt,  BinaryDataBGM::Jazz100E_txtSize},
            {100, "F",  "F",  BinaryDataBGM::Jazz100F_opus,  BinaryDataBGM::Jazz100F_opusSize,  BinaryDataBGM::Jazz100F_txt,  BinaryDataBGM::Jazz100F_txtSize},
            {100, "F#", "Fs", BinaryDataBGM::Jazz100Fs_opus, BinaryDataBGM::Jazz100Fs_opusSize, BinaryDataBGM::Jazz100Fs_txt, BinaryDataBGM::Jazz100Fs_txtSize},
            {100, "G",  "G",  BinaryDataBGM::Jazz100G_opus,  BinaryDataBGM::Jazz100G_opusSize,  BinaryDataBGM::Jazz100G_txt,  BinaryDataBGM::Jazz100G_txtSize},
            {100, "G#", "Gs", BinaryDataBGM::Jazz100Gs_opus, BinaryDataBGM::Jazz100Gs_opusSize, BinaryDataBGM::Jazz100Gs_txt, BinaryDataBGM::Jazz100Gs_txtSize},
            {100, "A",  "A",  BinaryDataBGM::Jazz100A_opus,  BinaryDataBGM::Jazz100A_opusSize,  BinaryDataBGM::Jazz100A_txt,  BinaryDataBGM::Jazz100A_txtSize},
            {100, "A#", "As", BinaryDataBGM::Jazz100As_opus, BinaryDataBGM::Jazz100As_opusSize, BinaryDataBGM::Jazz100As_txt, BinaryDataBGM::Jazz100As_txtSize},
            {100, "B",  "B",  BinaryDataBGM::Jazz100B_opus,  BinaryDataBGM::Jazz100B_opusSize,  BinaryDataBGM::Jazz100B_txt,  BinaryDataBGM::Jazz100B_txtSize},
            {120, "C",  "C",  BinaryDataBGM::Jazz120C_opus,  BinaryDataBGM::Jazz120C_opusSize,  BinaryDataBGM::Jazz120C_txt,  BinaryDataBGM::Jazz120C_txtSize},
            {120, "C#", "Cs", BinaryDataBGM::Jazz120Cs_opus, BinaryDataBGM::Jazz120Cs_opusSize, BinaryDataBGM::Jazz120Cs_txt, BinaryDataBGM::Jazz120Cs_txtSize},
            {120, "D",  "D",  BinaryDataBGM::Jazz120D_opus,  BinaryDataBGM::Jazz120D_opusSize,  BinaryDataBGM::Jazz120D_txt,  BinaryDataBGM::Jazz120D_txtSize},
            {120, "D#", "Ds", BinaryDataBGM::Jazz120Ds_opus, BinaryDataBGM::Jazz120Ds_opusSize, BinaryDataBGM::Jazz120Ds_txt, BinaryDataBGM::Jazz120Ds_txtSize},
            {120, "E",  "E",  BinaryDataBGM::Jazz120E_opus,  BinaryDataBGM::Jazz120E_opusSize,  BinaryDataBGM::Jazz120E_txt,  BinaryDataBGM::Jazz120E_txtSize},
            {120, "F",  "F",  BinaryDataBGM::Jazz120F_opus,  BinaryDataBGM::Jazz120F_opusSize,  BinaryDataBGM::Jazz120F_txt,  BinaryDataBGM::Jazz120F_txtSize},
            {120, "F#", "Fs", BinaryDataBGM::Jazz120Fs_opus, BinaryDataBGM::Jazz120Fs_opusSize, BinaryDataBGM::Jazz120Fs_txt, BinaryDataBGM::Jazz120Fs_txtSize},
            {120, "G",  "G",  BinaryDataBGM::Jazz120G_opus,  BinaryDataBGM::Jazz120G_opusSize,  BinaryDataBGM::Jazz120G_txt,  BinaryDataBGM::Jazz120G_txtSize},
            {120, "G#", "Gs", BinaryDataBGM::Jazz120Gs_opus, BinaryDataBGM::Jazz120Gs_opusSize, BinaryDataBGM::Jazz120Gs_txt, BinaryDataBGM::Jazz120Gs_txtSize},
            {120, "A",  "A",  BinaryDataBGM::Jazz120A_opus,  BinaryDataBGM::Jazz120A_opusSize,  BinaryDataBGM::Jazz120A_txt,  BinaryDataBGM::Jazz120A_txtSize},
            {120, "A#", "As", BinaryDataBGM::Jazz120As_opus, BinaryDataBGM::Jazz120As_opusSize, BinaryDataBGM::Jazz120As_txt, BinaryDataBGM::Jazz120As_txtSize},
            {120, "B",  "B",  BinaryDataBGM::Jazz120B_opus,  BinaryDataBGM::Jazz120B_opusSize,  BinaryDataBGM::Jazz120B_txt,  BinaryDataBGM::Jazz120B_txtSize},
            {140, "C",  "C",  BinaryDataBGM::Jazz140C_opus,  BinaryDataBGM::Jazz140C_opusSize,  BinaryDataBGM::Jazz140C_txt,  BinaryDataBGM::Jazz140C_txtSize},
            {140, "C#", "Cs", BinaryDataBGM::Jazz140Cs_opus, BinaryDataBGM::Jazz140Cs_opusSize, BinaryDataBGM::Jazz140Cs_txt, BinaryDataBGM::Jazz140Cs_txtSize},
            {140, "D",  "D",  BinaryDataBGM::Jazz140D_opus,  BinaryDataBGM::Jazz140D_opusSize,  BinaryDataBGM::Jazz140D_txt,  BinaryDataBGM::Jazz140D_txtSize},
            {140, "D#", "Ds", BinaryDataBGM::Jazz140Ds_opus, BinaryDataBGM::Jazz140Ds_opusSize, BinaryDataBGM::Jazz140Ds_txt, BinaryDataBGM::Jazz140Ds_txtSize},
            {140, "E",  "E",  BinaryDataBGM::Jazz140E_opus,  BinaryDataBGM::Jazz140E_opusSize,  BinaryDataBGM::Jazz140E_txt,  BinaryDataBGM::Jazz140E_txtSize},
            {140, "F",  "F",  BinaryDataBGM::Jazz140F_opus,  BinaryDataBGM::Jazz140F_opusSize,  BinaryDataBGM::Jazz140F_txt,  BinaryDataBGM::Jazz140F_txtSize},
            {140, "F#", "Fs", BinaryDataBGM::Jazz140Fs_opus, BinaryDataBGM::Jazz140Fs_opusSize, BinaryDataBGM::Jazz140Fs_txt, BinaryDataBGM::Jazz140Fs_txtSize},
            {140, "G",  "G",  BinaryDataBGM::Jazz140G_opus,  BinaryDataBGM::Jazz140G_opusSize,  BinaryDataBGM::Jazz140G_txt,  BinaryDataBGM::Jazz140G_txtSize},
            {140, "G#", "Gs", BinaryDataBGM::Jazz140Gs_opus, BinaryDataBGM::Jazz140Gs_opusSize, BinaryDataBGM::Jazz140Gs_txt, BinaryDataBGM::Jazz140Gs_txtSize},
            {140, "A",  "A",  BinaryDataBGM::Jazz140A_opus,  BinaryDataBGM::Jazz140A_opusSize,  BinaryDataBGM::Jazz140A_txt,  BinaryDataBGM::Jazz140A_txtSize},
            {140, "A#", "As", BinaryDataBGM::Jazz140As_opus, BinaryDataBGM::Jazz140As_opusSize, BinaryDataBGM::Jazz140As_txt, BinaryDataBGM::Jazz140As_txtSize},
            {140, "B",  "B",  BinaryDataBGM::Jazz140B_opus,  BinaryDataBGM::Jazz140B_opusSize,  BinaryDataBGM::Jazz140B_txt,  BinaryDataBGM::Jazz140B_txtSize},
        };
        // clang-format on

        std::vector<Entry> result;
        result.reserve(std::size(rows));
        for (const auto &row : rows) {
            Entry e;
            e.tempo = row.tempo;
            e.key = row.displayKey;
            e.entry.tempo = row.tempo;
            e.entry.key = row.internalKey;
            e.entry.displayKey = row.displayKey;
            e.entry.audioData = row.audioData;
            e.entry.audioSize = row.audioSize;
            e.entry.timingData = row.timingData;
            e.entry.timingSize = row.timingSize;
            result.push_back(e);
        }
        return result;
    }();

    for (const auto &e : table) {
        if (e.tempo == tempo && e.key == key)
            return &e.entry;
    }
    return nullptr;
}

} // namespace BuiltinResources
