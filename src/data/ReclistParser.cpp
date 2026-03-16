// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ReclistParser.h"

#include "../utils/TextEncoding.h"

juce::String ReclistParser::decodeBuffer(const void *data, size_t size)
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
    return ReclistParser::decodeBuffer(data.getData(), data.getSize());
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

std::optional<ReclistData> ReclistParser::loadFromMemory(const void *reclistData, size_t reclistSize,
                                                         const juce::String &name,
                                                         const void *commentData, size_t commentSize)
{
    auto text = decodeBuffer(reclistData, reclistSize);
    if (text.isEmpty())
        return std::nullopt;

    juce::StringArray tokens;
    tokens.addTokens(text, " \t\r\n", "");
    tokens.removeEmptyStrings();

    if (tokens.isEmpty())
        return std::nullopt;

    ReclistData result;
    result.name = name;
    for (const auto &t : tokens)
        result.entries.push_back(t);

    if (commentData != nullptr && commentSize > 0)
        result.comments = parseCommentText(decodeBuffer(commentData, commentSize));

    return result;
}

std::map<juce::String, juce::String> ReclistParser::parseCommentText(const juce::String &text)
{
    std::map<juce::String, juce::String> result;
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

std::map<juce::String, juce::String> ReclistParser::parseCommentFile(const juce::File &commentFile)
{
    return parseCommentText(readFileDecoded(commentFile));
}
