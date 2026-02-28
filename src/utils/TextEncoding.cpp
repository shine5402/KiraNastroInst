#include "TextEncoding.h"

#if JUCE_WINDOWS
#include <windows.h>
#elif JUCE_MAC || JUCE_IOS
#include <CoreFoundation/CoreFoundation.h>
#else
#include <cerrno>
#include <iconv.h>
#endif

#include <vector>

namespace TextEncoding
{
Encoding detectEncoding(const juce::MemoryBlock &data)
{
    auto *bytes = static_cast<const uint8_t *>(data.getData());
    const size_t size = data.getSize();

    if (size == 0)
        return Encoding::UTF8;

    // UTF-8 BOM: EF BB BF
    if (size >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
        return Encoding::UTF8;

    // Strict UTF-8 validation
    bool isValidUTF8 = true;
    size_t i = 0;
    while (i < size) {
        uint8_t b = bytes[i];
        int continuationBytes = 0;

        if (b <= 0x7F) {
            ++i;
            continue;
        }
        else if ((b & 0xE0) == 0xC0) {
            if (b < 0xC2) {
                isValidUTF8 = false;
                break;
            } // overlong
            continuationBytes = 1;
        }
        else if ((b & 0xF0) == 0xE0) {
            continuationBytes = 2;
        }
        else if ((b & 0xF8) == 0xF0) {
            if (b > 0xF4) {
                isValidUTF8 = false;
                break;
            }
            continuationBytes = 3;
        }
        else {
            isValidUTF8 = false;
            break;
        }

        for (int j = 1; j <= continuationBytes; ++j) {
            if (i + static_cast<size_t>(j) >= size || (bytes[i + static_cast<size_t>(j)] & 0xC0) != 0x80) {
                isValidUTF8 = false;
                break;
            }
        }
        if (!isValidUTF8)
            break;
        i += 1 + static_cast<size_t>(continuationBytes);
    }

    if (isValidUTF8)
        return Encoding::UTF8;

    // Shift-JIS heuristic
    bool hasValidSJISDouble = false;
    i = 0;
    while (i < size) {
        uint8_t b = bytes[i];

        if (b <= 0x7F) {
            ++i;
            continue;
        }

        // Half-width katakana: single byte
        if (b >= 0xA1 && b <= 0xDF) {
            ++i;
            continue;
        }

        // Shift-JIS lead byte
        if ((b >= 0x81 && b <= 0x9F) || (b >= 0xE0 && b <= 0xEF)) {
            if (i + 1 >= size)
                return Encoding::Unknown;
            uint8_t trail = bytes[i + 1];
            if ((trail >= 0x40 && trail <= 0x7E) || (trail >= 0x80 && trail <= 0xFC)) {
                hasValidSJISDouble = true;
                i += 2;
                continue;
            }
            return Encoding::Unknown;
        }

        // Invalid byte for Shift-JIS
        return Encoding::Unknown;
    }

    return hasValidSJISDouble ? Encoding::ShiftJIS : Encoding::Unknown;
}

juce::String shiftJisToString(const void *data, size_t numBytes)
{
    if (data == nullptr || numBytes == 0)
        return {};

#if JUCE_WINDOWS
    int wideLen = MultiByteToWideChar(932, 0, static_cast<LPCCH>(data), static_cast<int>(numBytes), nullptr, 0);
    if (wideLen <= 0)
        return {};
    std::vector<wchar_t> wbuf(static_cast<size_t>(wideLen + 1), 0);
    MultiByteToWideChar(932, 0, static_cast<LPCCH>(data), static_cast<int>(numBytes), wbuf.data(), wideLen);
    return juce::String(wbuf.data());

#elif JUCE_MAC || JUCE_IOS
    CFStringRef cfStr = CFStringCreateWithBytes(kCFAllocatorDefault, static_cast<const UInt8 *>(data),
                                                static_cast<CFIndex>(numBytes), kCFStringEncodingDOSJapanese, false);
    if (cfStr == nullptr)
        return {};
    CFIndex utf8Len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfStr), kCFStringEncodingUTF8) + 1;
    std::vector<char> utf8buf(static_cast<size_t>(utf8Len), 0);
    CFStringGetCString(cfStr, utf8buf.data(), utf8Len, kCFStringEncodingUTF8);
    CFRelease(cfStr);
    return juce::String::fromUTF8(utf8buf.data());

#else
    // Linux/other: use iconv
    iconv_t cd = iconv_open("UTF-8", "SHIFT-JIS");
    if (cd == (iconv_t)-1)
        return {};
    size_t inBytesLeft = numBytes;
    size_t outBufSize = numBytes * 4 + 1;
    std::vector<char> outBuf(outBufSize, 0);
    char *inPtr = const_cast<char *>(static_cast<const char *>(data));
    char *outPtr = outBuf.data();
    size_t outBytesLeft = outBufSize - 1;
    while (inBytesLeft > 0) {
        size_t result = iconv(cd, &inPtr, &inBytesLeft, &outPtr, &outBytesLeft);
        if (result == static_cast<size_t>(-1)) {
            if (errno == EILSEQ || errno == EINVAL) {
                // Skip invalid byte and continue
                ++inPtr;
                --inBytesLeft;
            }
            else
                break;
        }
    }
    iconv_close(cd);
    return juce::String::fromUTF8(outBuf.data());
#endif
}

} // namespace TextEncoding
