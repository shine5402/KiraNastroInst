// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>
#include "utils/TextEncoding.h"

// Helper: make a MemoryBlock from a raw byte array
template <size_t N>
static juce::MemoryBlock mb(const uint8_t (&bytes)[N])
{
    return juce::MemoryBlock(bytes, N);
}

// ── detectEncoding ────────────────────────────────────────────────────────────

TEST_CASE("detectEncoding: empty data -> UTF8", "[TextEncoding]")
{
    juce::MemoryBlock empty;
    CHECK(TextEncoding::detectEncoding(empty) == TextEncoding::Encoding::UTF8);
}

TEST_CASE("detectEncoding: pure ASCII -> UTF8", "[TextEncoding]")
{
    const char* s = "hello world\n";
    juce::MemoryBlock data(s, strlen(s));
    CHECK(TextEncoding::detectEncoding(data) == TextEncoding::Encoding::UTF8);
}

TEST_CASE("detectEncoding: UTF-8 BOM -> UTF8", "[TextEncoding]")
{
    // EF BB BF (BOM) + "hi"
    const uint8_t bytes[] = { 0xEF, 0xBB, 0xBF, 0x68, 0x69 };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::UTF8);
}

TEST_CASE("detectEncoding: valid 3-byte UTF-8 sequence -> UTF8", "[TextEncoding]")
{
    // U+3042 "あ" in UTF-8: E3 81 82
    const uint8_t bytes[] = { 0xE3, 0x81, 0x82 };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::UTF8);
}

TEST_CASE("detectEncoding: valid 2-byte UTF-8 sequence -> UTF8", "[TextEncoding]")
{
    // U+00E9 "é" in UTF-8: C3 A9
    const uint8_t bytes[] = { 0xC3, 0xA9 };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::UTF8);
}

TEST_CASE("detectEncoding: Shift-JIS double-byte sequence -> ShiftJIS", "[TextEncoding]")
{
    // Two valid SJIS double-byte chars. Lead bytes in 0x81–0x9F, trail in 0x40–0xFC.
    // 0x82 0x50 and 0x93 0xFA are valid lead+trail combinations.
    const uint8_t bytes[] = { 0x82, 0x50, 0x93, 0xFA };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::ShiftJIS);
}

TEST_CASE("detectEncoding: SJIS lead with invalid trail -> Unknown", "[TextEncoding]")
{
    // 0x82 is valid lead, but 0x01 is not a valid trail byte
    const uint8_t bytes[] = { 0x82, 0x01 };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::Unknown);
}

TEST_CASE("detectEncoding: isolated high byte (not ASCII, not SJIS, not UTF-8) -> Unknown",
          "[TextEncoding]")
{
    // 0x80 is not a valid UTF-8 lead, SJIS lead, or known BOM byte
    const uint8_t bytes[] = { 0x80, 0x80 };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::Unknown);
}

// ── UTF-16 BOM detection ───────────────────────────────────────────────────────

TEST_CASE("detectEncoding: UTF-16 LE BOM (FF FE) -> UTF16LE", "[TextEncoding]")
{
    // FF FE followed by 'h' 'i' in LE encoding
    const uint8_t bytes[] = { 0xFF, 0xFE, 0x68, 0x00, 0x69, 0x00 };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::UTF16LE);
}

TEST_CASE("detectEncoding: UTF-16 BE BOM (FE FF) -> UTF16BE", "[TextEncoding]")
{
    // FE FF followed by 'h' 'i' in BE encoding
    const uint8_t bytes[] = { 0xFE, 0xFF, 0x00, 0x68, 0x00, 0x69 };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::UTF16BE);
}

TEST_CASE("detectEncoding: UTF-16 LE BOM only (2 bytes) -> UTF16LE", "[TextEncoding]")
{
    const uint8_t bytes[] = { 0xFF, 0xFE };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::UTF16LE);
}

TEST_CASE("detectEncoding: UTF-16 BE BOM only (2 bytes) -> UTF16BE", "[TextEncoding]")
{
    const uint8_t bytes[] = { 0xFE, 0xFF };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::UTF16BE);
}

// ── utf16ToString ──────────────────────────────────────────────────────────────

TEST_CASE("utf16ToString: too short (< 4 bytes) -> empty", "[TextEncoding]")
{
    const uint8_t bom_only[] = { 0xFF, 0xFE };
    CHECK(TextEncoding::utf16ToString(bom_only, sizeof(bom_only), false).isEmpty());

    const uint8_t three_bytes[] = { 0xFF, 0xFE, 0x68 };
    CHECK(TextEncoding::utf16ToString(three_bytes, sizeof(three_bytes), false).isEmpty());
}

TEST_CASE("utf16ToString: UTF-16 LE ASCII string", "[TextEncoding]")
{
    // BOM + "hi" in UTF-16 LE
    const uint8_t bytes[] = { 0xFF, 0xFE, 0x68, 0x00, 0x69, 0x00 };
    CHECK(TextEncoding::utf16ToString(bytes, sizeof(bytes), false) == juce::String("hi"));
}

TEST_CASE("utf16ToString: UTF-16 BE ASCII string", "[TextEncoding]")
{
    // BOM + "hi" in UTF-16 BE
    const uint8_t bytes[] = { 0xFE, 0xFF, 0x00, 0x68, 0x00, 0x69 };
    CHECK(TextEncoding::utf16ToString(bytes, sizeof(bytes), true) == juce::String("hi"));
}

TEST_CASE("utf16ToString: UTF-16 LE Japanese character U+3042", "[TextEncoding]")
{
    // BOM + U+3042 in UTF-16 LE: 42 30
    const uint8_t bytes[] = { 0xFF, 0xFE, 0x42, 0x30 };
    auto result = TextEncoding::utf16ToString(bytes, sizeof(bytes), false);
    CHECK(result == juce::String::fromUTF8("\xe3\x81\x82")); // "あ" in UTF-8
}

TEST_CASE("utf16ToString: UTF-16 BE Japanese character U+3042", "[TextEncoding]")
{
    // BOM + U+3042 in UTF-16 BE: 30 42
    const uint8_t bytes[] = { 0xFE, 0xFF, 0x30, 0x42 };
    auto result = TextEncoding::utf16ToString(bytes, sizeof(bytes), true);
    CHECK(result == juce::String::fromUTF8("\xe3\x81\x82")); // "あ" in UTF-8
}

TEST_CASE("utf16ToString: LE and BE produce identical strings for same content", "[TextEncoding]")
{
    // "ok" in UTF-16 LE (BOM + 6F 00 6B 00)
    const uint8_t le[] = { 0xFF, 0xFE, 0x6F, 0x00, 0x6B, 0x00 };
    // "ok" in UTF-16 BE (BOM + 00 6F 00 6B)
    const uint8_t be[] = { 0xFE, 0xFF, 0x00, 0x6F, 0x00, 0x6B };
    CHECK(TextEncoding::utf16ToString(le, sizeof(le), false) ==
          TextEncoding::utf16ToString(be, sizeof(be), true));
}

TEST_CASE("detectEncoding: SJIS half-width katakana (single-byte 0xA1-0xDF) -> ShiftJIS",
          "[TextEncoding]")
{
    // Half-width katakana are valid single-byte SJIS chars, but alone they aren't
    // enough to trigger ShiftJIS (no double-byte sequence). Combine with one double-byte.
    const uint8_t bytes[] = { 0xA1, 0x82, 0x50 }; // katakana + one double-byte pair
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::ShiftJIS);
}

TEST_CASE("detectEncoding: truncated SJIS sequence at end of buffer -> Unknown", "[TextEncoding]")
{
    // Lead byte with no trail byte following
    const uint8_t bytes[] = { 0x82 };
    CHECK(TextEncoding::detectEncoding(mb(bytes)) == TextEncoding::Encoding::Unknown);
}

// ── shiftJisToString ──────────────────────────────────────────────────────────

TEST_CASE("shiftJisToString: null/empty input -> empty string", "[TextEncoding]")
{
    CHECK(TextEncoding::shiftJisToString(nullptr, 0).isEmpty());
    CHECK(TextEncoding::shiftJisToString("x", 0).isEmpty());
}

TEST_CASE("shiftJisToString: ASCII passthrough", "[TextEncoding]")
{
    const char* s = "hello, world!";
    auto result = TextEncoding::shiftJisToString(s, strlen(s));
    CHECK(result == juce::String("hello, world!"));
}

TEST_CASE("shiftJisToString: valid Shift-JIS returns non-empty string", "[TextEncoding]")
{
    // 0x93 0xFA is a valid CP932 double-byte sequence
    const uint8_t bytes[] = { 0x93, 0xFA };
    auto result = TextEncoding::shiftJisToString(bytes, sizeof(bytes));
    CHECK_FALSE(result.isEmpty());
}
