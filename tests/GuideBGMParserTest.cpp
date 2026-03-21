// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "data/GuideBGMParser.h"

// Bundled test data — see tests/testdata/NOTES.md for license info
// Shift-JIS originals (marked binary in .gitattributes); UTF-8 copy alongside.
static constexpr const char* kJazzOpusPath    = TEST_DATA_DIR "/Jazz-100-A.opus";
static constexpr const char* kJazzTxtPath     = TEST_DATA_DIR "/Jazz-100-A.txt";
static constexpr const char* kJazzTxtUtf8Path = TEST_DATA_DIR "/Jazz-100-A-utf8.txt";

// ── Error cases ───────────────────────────────────────────────────────────────

TEST_CASE("GuideBGMParser: non-existent timing file -> nullopt", "[GuideBGMParser]")
{
    auto result = GuideBGMParser::loadFromTimingFile(juce::File("/does/not/exist.txt"));
    CHECK_FALSE(result.has_value());
}

TEST_CASE("GuideBGMParser: load() with non-existent wav -> nullopt", "[GuideBGMParser]")
{
    auto result = GuideBGMParser::load(juce::File("/does/not/exist.wav"));
    CHECK_FALSE(result.has_value());
}

TEST_CASE("GuideBGMParser: rejects file with wrong/missing unit line", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText("beats\n1, 1.0, 0, 0, 0, 0\n", false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    CHECK_FALSE(result.has_value());
}

TEST_CASE("GuideBGMParser: rejects file with fewer than 6 data rows", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText("msec\n1, 1000.0, 1, 0, 0, 0\n2, 2000.0, 0, 1, 0, 0\n",
                                  false, false, "\n");
    CHECK_FALSE(GuideBGMParser::loadFromTimingFile(tmp.getFile()).has_value());
}

// ── Real Jazz-100-A.txt (6 rows, msec unit) ──────────────────────────────────

TEST_CASE("GuideBGMParser: Jazz-100-A.txt - named timing fields", "[GuideBGMParser]")
{
    juce::File f(kJazzTxtPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.txt not found");

    auto result = GuideBGMParser::loadFromTimingFile(f);
    REQUIRE(result.has_value());
    CHECK(result->name == "Jazz-100-A");

    const auto& t = result->timing;
    CHECK(t.bgmPlaybackStartMs == Catch::Approx(9600.0));
    CHECK(t.recordingStartMs   == Catch::Approx(10800.0));
    CHECK(t.utteranceStartMs   == Catch::Approx(12000.0));
    CHECK(t.utteranceEndMs     == Catch::Approx(16800.0));
    CHECK(t.recordingEndMs     == Catch::Approx(18000.0));
    CHECK(t.bgmLoopMs          == Catch::Approx(19200.0));

    // All six times strictly increasing
    CHECK(t.bgmPlaybackStartMs < t.recordingStartMs);
    CHECK(t.recordingStartMs   < t.utteranceStartMs);
    CHECK(t.utteranceStartMs   < t.utteranceEndMs);
    CHECK(t.utteranceEndMs     < t.recordingEndMs);
    CHECK(t.recordingEndMs     < t.bgmLoopMs);
}

TEST_CASE("GuideBGMParser: load() delegates to sibling .txt of audio file", "[GuideBGMParser]")
{
    juce::File f(kJazzOpusPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.opus not found");

    auto result = GuideBGMParser::load(f);
    REQUIRE(result.has_value());
    CHECK(result->timing.bgmPlaybackStartMs == Catch::Approx(9600.0));
    CHECK(result->timing.bgmLoopMs          == Catch::Approx(19200.0));
}

// ── Synthetic in-memory parsing ───────────────────────────────────────────────

TEST_CASE("GuideBGMParser: msec unit - times stored as-is", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText(
        "msec\n"
        "1,  9600.0, 0, 0, 0, 0\n"
        "2, 10800.0, 1, 0, 0, 0\n"
        "3, 12000.0, 0, 0, 0, 0\n"
        "4, 16800.0, 0, 0, 0, 0\n"
        "5, 18000.0, 0, 1, 0, 0\n"
        "6, 19200.0, 0, 0, 1, 1\n",
        false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    REQUIRE(result.has_value());
    CHECK(result->timing.bgmPlaybackStartMs == Catch::Approx(9600.0));
    CHECK(result->timing.bgmLoopMs          == Catch::Approx(19200.0));
}

TEST_CASE("GuideBGMParser: sec unit - times converted to milliseconds", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText(
        "sec\n"
        "1,  9.6, 0, 0, 0, 0\n"
        "2, 10.8, 1, 0, 0, 0\n"
        "3, 12.0, 0, 0, 0, 0\n"
        "4, 16.8, 0, 0, 0, 0\n"
        "5, 18.0, 0, 1, 0, 0\n"
        "6, 19.2, 0, 0, 1, 1\n",
        false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    REQUIRE(result.has_value());
    CHECK(result->timing.bgmPlaybackStartMs == Catch::Approx(9600.0));
    CHECK(result->timing.bgmLoopMs          == Catch::Approx(19200.0));
}

TEST_CASE("GuideBGMParser: '#' comment lines are skipped", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText(
        "msec\n"
        "# comment\n"
        "1,  9600.0, 0, 0, 0, 0, BGM start\n"
        "2, 10800.0, 1, 0, 0, 0\n"
        "3, 12000.0, 0, 0, 0, 0\n"
        "4, 16800.0, 0, 0, 0, 0\n"
        "5, 18000.0, 0, 1, 0, 0\n"
        "6, 19200.0, 0, 0, 1, 1\n",
        false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    REQUIRE(result.has_value());
    CHECK(result->timing.bgmPlaybackStartMs == Catch::Approx(9600.0));
}

// ── Encoding tests ────────────────────────────────────────────────────────────

TEST_CASE("GuideBGMParser: UTF-8 and Shift-JIS versions decode to identical timing",
          "[GuideBGMParser][encoding]")
{
    juce::File sjisFile(kJazzTxtPath);
    juce::File utf8File(kJazzTxtUtf8Path);
    if (!sjisFile.existsAsFile() || !utf8File.existsAsFile())
        SKIP("test data not found");

    auto sjis = GuideBGMParser::loadFromTimingFile(sjisFile);
    auto utf8 = GuideBGMParser::loadFromTimingFile(utf8File);

    REQUIRE(sjis.has_value());
    REQUIRE(utf8.has_value());
    CHECK(sjis->timing.bgmPlaybackStartMs == Catch::Approx(utf8->timing.bgmPlaybackStartMs));
    CHECK(sjis->timing.recordingStartMs   == Catch::Approx(utf8->timing.recordingStartMs));
    CHECK(sjis->timing.utteranceStartMs   == Catch::Approx(utf8->timing.utteranceStartMs));
    CHECK(sjis->timing.utteranceEndMs     == Catch::Approx(utf8->timing.utteranceEndMs));
    CHECK(sjis->timing.recordingEndMs     == Catch::Approx(utf8->timing.recordingEndMs));
    CHECK(sjis->timing.bgmLoopMs          == Catch::Approx(utf8->timing.bgmLoopMs));
}
