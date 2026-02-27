#pragma once

#include <juce_core/juce_core.h>

// Phase 5: Tracks per-entry recording timestamps and exports them as
// a KiraWavTar-compatible .kirawavtar-desc.json label file.
class LabelExporter
{
public:
    LabelExporter() = default;
    ~LabelExporter() = default;

    // TODO Phase 5: implement addEntry(name, startSec, endSec), exportToFile(File)

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabelExporter)
};
