#include "GuideBGMParser.h"
#include "../utils/TextEncoding.h"

static juce::String readFileDecoded(const juce::File& file)
{
    juce::MemoryBlock data;
    if (!file.loadFileAsData(data) || data.isEmpty()) return {};
    auto enc = TextEncoding::detectEncoding(data);
    if (enc == TextEncoding::Encoding::ShiftJIS)
        return TextEncoding::shiftJisToString(data.getData(), data.getSize());
    return juce::String::fromUTF8(
        static_cast<const char*>(data.getData()), static_cast<int>(data.getSize()));
}

std::optional<GuideBGMData> GuideBGMParser::load(const juce::File& wavFile)
{
    return loadFromTimingFile(wavFile.withFileExtension("txt"));
}

std::optional<GuideBGMData> GuideBGMParser::loadFromTimingFile(const juce::File& timingFile)
{
    auto text = readFileDecoded(timingFile);
    if (text.isEmpty()) return std::nullopt;

    auto lines = juce::StringArray::fromLines(text);
    if (lines.isEmpty()) return std::nullopt;

    // Line 0: time unit declaration ("sec" or "msec")
    auto unitLine = lines[0].trim();
    double timeMultiplier = 0.0;
    if (unitLine == "msec")
        timeMultiplier = 1.0;
    else if (unitLine == "sec")
        timeMultiplier = 1000.0;
    else
        return std::nullopt;

    GuideBGMData result;
    result.name = timingFile.getFileNameWithoutExtension();

    for (int lineIdx = 1; lineIdx < lines.size(); ++lineIdx)
    {
        auto line = lines[lineIdx].trim();
        if (line.isEmpty() || line.startsWith("#"))
            continue;

        juce::StringArray fields;
        fields.addTokens(line, ",", "");
        if (fields.size() < 6)
            continue;

        // Field 0: row number (1-indexed, not stored)
        // Field 1: time position
        double rawTime = fields[1].trim().getDoubleValue();
        double timeMs  = rawTime * timeMultiplier;

        // Fields 2-4: boolean flags
        bool isRecStart = (fields[2].trim().getIntValue() == 1);
        bool isRecEnd   = (fields[3].trim().getIntValue() == 1);
        bool isSwitching = (fields[4].trim().getIntValue() == 1);

        // Field 5: repeat target (1-indexed in file; convert to 0-indexed, -1 = no repeat)
        int rawRepeat = fields[5].trim().getIntValue();
        int repeatTargetNodeIndex = rawRepeat > 0 ? rawRepeat - 1 : -1;

        // Fields 6+: optional comment (may contain commas — rejoin)
        juce::String comment;
        if (fields.size() > 6)
        {
            for (int f = 6; f < fields.size(); ++f)
            {
                if (f > 6) comment += ",";
                comment += fields[f];
            }
            comment = comment.trim();
        }

        TimingNode node;
        node.timeMs               = timeMs;
        node.isRecordingStart     = isRecStart;
        node.isRecordingEnd       = isRecEnd;
        node.isSwitching          = isSwitching;
        node.repeatTargetNodeIndex = repeatTargetNodeIndex;
        node.comment              = comment;

        result.nodes.push_back(node);
    }

    if (result.nodes.empty()) return std::nullopt;
    return result;
}
