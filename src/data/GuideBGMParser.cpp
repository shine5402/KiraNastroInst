// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GuideBGMParser.h"

#include "../utils/TextEncoding.h"

static juce::String readFileDecoded(const juce::File &file)
{
    juce::MemoryBlock data;
    if (!file.loadFileAsData(data) || data.isEmpty())
        return {};
    auto enc = TextEncoding::detectEncoding(data);
    if (enc == TextEncoding::Encoding::ShiftJIS)
        return TextEncoding::shiftJisToString(data.getData(), data.getSize());
    return juce::String::fromUTF8(static_cast<const char *>(data.getData()), static_cast<int>(data.getSize()));
}

std::optional<GuideBGMData> GuideBGMParser::load(const juce::File &wavFile)
{
    return loadFromTimingFile(wavFile.withFileExtension("txt"));
}

std::optional<GuideBGMData> GuideBGMParser::loadFromTimingFile(const juce::File &timingFile)
{
    auto text = readFileDecoded(timingFile);
    if (text.isEmpty())
        return std::nullopt;

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
    result.name = timingFile.getFileNameWithoutExtension();
    result.timing.bgmPlaybackStartMs = times[0];
    result.timing.recordingStartMs   = times[1];
    result.timing.utteranceStartMs   = times[2];
    result.timing.utteranceEndMs     = times[3];
    result.timing.recordingEndMs     = times[4];
    result.timing.bgmLoopMs          = times[5];
    return result;
}
