#include "LabelExporter.h"

juce::String LabelExporter::toTimecode(double seconds)
{
    int h = static_cast<int>(seconds / 3600.0);
    int m = static_cast<int>((seconds - h * 3600.0) / 60.0);
    double s = seconds - h * 3600.0 - m * 60.0;
    return juce::String::formatted("%02d:%02d:%06.3f", h, m, s);
}

bool LabelExporter::exportToFile(const juce::File &destFile, const KiraNastroProcessor::DescExportParams &params,
                                 juce::String &errorMessage)
{
    if (!params.isValid()) {
        errorMessage = "Export parameters are invalid: ensure both a reclist and "
                       "a guide BGM with a recording window are loaded.";
        return false;
    }

    const int numEntries = static_cast<int>(params.entryNames.size());
    const double blockDur = params.blockDurationSec;
    const double recOffset = params.recordingStartOffsetSec;
    const double recDur = params.recordingWindowDurationSec;
    const double totalDur = numEntries * blockDur + recOffset + recDur;
    const int sampleRate = static_cast<int>(std::round(params.sampleRate));

    // Build root object
    auto *rootObj = new juce::DynamicObject();
    rootObj->setProperty("version", juce::var(4));
    rootObj->setProperty("combiner", juce::var(juce::String("nastro_inst")));
    rootObj->setProperty("sample_rate", juce::var(sampleRate));
    rootObj->setProperty("sample_type", juce::var(2));      // i16
    rootObj->setProperty("channel_count", juce::var(1));    // mono
    rootObj->setProperty("container_format", juce::var(0)); // RIFF WAV
    rootObj->setProperty("gap_duration", juce::var(juce::String("00:00:00.000")));
    rootObj->setProperty("total_duration", juce::var(toTimecode(totalDur)));

    // Build descriptions array
    juce::Array<juce::var> descriptions;
    descriptions.ensureStorageAllocated(numEntries);

    for (int i = 0; i < numEntries; ++i) {
        double beginTime = i * blockDur + recOffset;

        auto *entryObj = new juce::DynamicObject();
        entryObj->setProperty("file_name", juce::var(params.entryNames[static_cast<size_t>(i)] + ".wav"));
        entryObj->setProperty("sample_rate", juce::var(sampleRate));
        entryObj->setProperty("sample_type", juce::var(2));
        entryObj->setProperty("channel_count", juce::var(1));
        entryObj->setProperty("container_format", juce::var(0));
        entryObj->setProperty("begin_time", juce::var(toTimecode(beginTime)));
        entryObj->setProperty("duration", juce::var(toTimecode(recDur)));

        descriptions.add(juce::var(entryObj));
    }

    rootObj->setProperty("descriptions", juce::var(descriptions));

    juce::var rootVar(rootObj);
    juce::String jsonText = juce::JSON::toString(rootVar, false);

    if (!destFile.replaceWithText(jsonText, false, false, "\n")) {
        errorMessage = "Failed to write file: " + destFile.getFullPathName() +
                       "\nCheck that the directory exists and you have write permission.";
        return false;
    }

    return true;
}
