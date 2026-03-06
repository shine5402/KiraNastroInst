// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>

// Phase 2: Encoding detection and conversion utilities.
// The UTAU community widely uses Shift-JIS for reclist and timing files.
namespace TextEncoding
{
enum class Encoding
{
    UTF8,
    UTF16LE,
    UTF16BE,
    ShiftJIS,
    Unknown
};

// Heuristically detect the encoding of raw file bytes.
// Returns ShiftJIS if Shift-JIS byte patterns are detected and the
// content is not valid UTF-8; otherwise returns UTF8.
Encoding detectEncoding(const juce::MemoryBlock &data);

// Convert Shift-JIS encoded bytes to a JUCE String (UTF-8 internally).
juce::String shiftJisToString(const void *data, size_t numBytes);

// Convert UTF-16 LE or BE encoded bytes (including BOM) to a JUCE String.
juce::String utf16ToString(const void *data, size_t numBytes, bool bigEndian);
} // namespace TextEncoding
