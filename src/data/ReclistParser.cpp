// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ReclistParser.h"

#include "../utils/TextEncoding.h"

static juce::String readFileDecoded(const juce::File &file)
{
    juce::MemoryBlock data;
    if (!file.loadFileAsData(data) || data.isEmpty())
        return {};
    auto enc = TextEncoding::detectEncoding(data);
    switch (enc) {
        case TextEncoding::Encoding::UTF16LE:
            return TextEncoding::utf16ToString(data.getData(), data.getSize(), false);
        case TextEncoding::Encoding::UTF16BE:
            return TextEncoding::utf16ToString(data.getData(), data.getSize(), true);
        case TextEncoding::Encoding::ShiftJIS:
            return TextEncoding::shiftJisToString(data.getData(), data.getSize());
        case TextEncoding::Encoding::Unknown:
            return {};
        case TextEncoding::Encoding::UTF8:
        default:
            return juce::String::fromUTF8(static_cast<const char *>(data.getData()), static_cast<int>(data.getSize()));
    }
}

std::optional<ReclistData> ReclistParser::load(const juce::File &reclistFile)
{
    auto text = readFileDecoded(reclistFile);
    if (text.isEmpty())
        return std::nullopt;

    juce::StringArray tokens;
    tokens.addTokens(text, " \t\r\n", "");
    tokens.removeEmptyStrings();

    if (tokens.isEmpty())
        return std::nullopt;

    ReclistData result;
    result.name = reclistFile.getFileNameWithoutExtension();
    for (const auto &t : tokens)
        result.entries.push_back(t);

    auto commentFile = reclistFile.getSiblingFile(result.name + "-comment.txt");
    result.comments = parseCommentFile(commentFile);

    return result;
}

std::map<juce::String, juce::String> ReclistParser::parseCommentFile(const juce::File &commentFile)
{
    std::map<juce::String, juce::String> result;

    auto text = readFileDecoded(commentFile);
    if (text.isEmpty())
        return result;

    auto lines = juce::StringArray::fromLines(text);
    for (const auto &line : lines) {
        auto trimmed = line.trim();
        if (trimmed.isEmpty() || trimmed.startsWith("#"))
            continue;

        // Find first separator: space, tab, or colon
        int sepPos = -1;
        for (int i = 0; i < trimmed.length(); ++i) {
            auto c = trimmed[i];
            if (c == ' ' || c == '\t' || c == ':') {
                sepPos = i;
                break;
            }
        }
        if (sepPos < 0)
            continue;

        auto key = trimmed.substring(0, sepPos).trim();
        auto value = trimmed.substring(sepPos + 1).trim();
        if (key.isEmpty() || value.isEmpty())
            continue;

        result[key] = value;
    }
    return result;
}
