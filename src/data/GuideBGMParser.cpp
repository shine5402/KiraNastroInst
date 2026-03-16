// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GuideBGMParser.h"

#include "../utils/TextEncoding.h"

static juce::String decodeBuffer(const void *data, size_t size)
{
    if (data == nullptr || size == 0)
        return {};
    juce::MemoryBlock block(data, size);
    auto enc = TextEncoding::detectEncoding(block);
    switch (enc) {
        case TextEncoding::Encoding::UTF16LE:
            return TextEncoding::utf16ToString(block.getData(), block.getSize(), false);
        case TextEncoding::Encoding::UTF16BE:
            return TextEncoding::utf16ToString(block.getData(), block.getSize(), true);
        case TextEncoding::Encoding::ShiftJIS:
            return TextEncoding::shiftJisToString(block.getData(), block.getSize());
        case TextEncoding::Encoding::Unknown:
            return {};
        case TextEncoding::Encoding::UTF8:
        default:
            return juce::String::fromUTF8(static_cast<const char *>(block.getData()),
                                          static_cast<int>(block.getSize()));
    }
}

static juce::String readFileDecoded(const juce::File &file)
{
    juce::MemoryBlock data;
    if (!file.loadFileAsData(data) || data.isEmpty())
        return {};
    return decodeBuffer(data.getData(), data.getSize());
}

static std::optional<GuideBGMData> parseTimingText(const juce::String &text, const juce::String &name)
{
    auto lines = juce::StringArray::fromLines(text);
    if (lines.isEmpty())
        return std::nullopt;

    // Line 0: time unit declaration ("sec" or "msec")
    auto unitLine = lines[0].trim();
    double timeMultiplier = 0.0;
    if (unitLine == "msec")
        timeMultiplier = 1.0;
    else if (unitLine == "sec")
        timeMultiplier = 1000.0;
    else
        return std::nullopt;

    // Collect time values in order (one per data row)
    std::vector<double> times;
    times.reserve(6);

    for (int lineIdx = 1; lineIdx < lines.size(); ++lineIdx) {
        auto line = lines[lineIdx].trim();
        if (line.isEmpty() || line.startsWith("#"))
            continue;

        juce::StringArray fields;
        fields.addTokens(line, ",", "");
        if (fields.size() < 6)
            continue;

        times.push_back(fields[1].trim().getDoubleValue() * timeMultiplier);
    }

    // Require exactly 6 data rows (OREMO standard)
    if (times.size() != 6)
        return std::nullopt;

    GuideBGMData result;
    result.name = name;
    result.timing.bgmPlaybackStartMs = times[0];
    result.timing.recordingStartMs   = times[1];
    result.timing.utteranceStartMs   = times[2];
    result.timing.utteranceEndMs     = times[3];
    result.timing.recordingEndMs     = times[4];
    result.timing.bgmLoopMs          = times[5];
    return result;
}

std::optional<GuideBGMData> GuideBGMParser::load(const juce::File &audioFile)
{
    return loadFromTimingFile(audioFile.withFileExtension("txt"));
}

std::optional<GuideBGMData> GuideBGMParser::loadFromTimingFile(const juce::File &timingFile)
{
    auto text = readFileDecoded(timingFile);
    if (text.isEmpty())
        return std::nullopt;
    return parseTimingText(text, timingFile.getFileNameWithoutExtension());
}

std::optional<GuideBGMData> GuideBGMParser::loadFromMemory(const void *timingData, size_t timingSize,
                                                            const juce::String &name)
{
    auto text = decodeBuffer(timingData, timingSize);
    if (text.isEmpty())
        return std::nullopt;
    return parseTimingText(text, name);
}
