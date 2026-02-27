#pragma once

#include <juce_core/juce_core.h>

// Phase 2: Encoding detection and conversion utilities.
// The UTAU community widely uses Shift-JIS for reclist and timing files.
namespace TextEncoding
{
    enum class Encoding { UTF8, ShiftJIS, Unknown };

    // Heuristically detect the encoding of raw file bytes.
    // Returns ShiftJIS if Shift-JIS byte patterns are detected and the
    // content is not valid UTF-8; otherwise returns UTF8.
    Encoding detectEncoding(const juce::MemoryBlock& data);

    // Convert Shift-JIS encoded bytes to a JUCE String (UTF-8 internally).
    juce::String shiftJisToString(const void* data, size_t numBytes);
} // namespace TextEncoding
