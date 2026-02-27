#include "TextEncoding.h"

namespace TextEncoding
{
    Encoding detectEncoding(const juce::MemoryBlock& /*data*/)
    {
        // TODO Phase 2: implement UTF-8 validation, then Shift-JIS heuristic
        return Encoding::UTF8;
    }

    juce::String shiftJisToString(const void* /*data*/, size_t /*numBytes*/)
    {
        // TODO Phase 2: use iconv or a lookup table to convert Shift-JIS → UTF-8
        return {};
    }
} // namespace TextEncoding
