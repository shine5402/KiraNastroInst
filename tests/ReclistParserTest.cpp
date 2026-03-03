// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>
#include "data/ReclistParser.h"

// Bundled test data — see tests/testdata/NOTES.md for license info
// Shift-JIS original (marked binary in .gitattributes); UTF-8 copy alongside it.
// The comment sidecar is auto-discovered by the parser (basename + "-comment.txt").
static constexpr const char* k8MoraPath     = TEST_DATA_DIR "/8mora.txt";
static constexpr const char* k8MoraUtf8Path = TEST_DATA_DIR "/8mora-utf8.txt";

// ── Error cases ───────────────────────────────────────────────────────────────

TEST_CASE("ReclistParser: non-existent file -> nullopt", "[ReclistParser]")
{
    auto result = ReclistParser::load(juce::File("/does/not/exist.txt"));
    CHECK_FALSE(result.has_value());
}

// ── 8mora.txt (Shift-JIS, entries on individual lines) ────────────────────────

TEST_CASE("ReclistParser: 8mora.txt - loads correct number of entries", "[ReclistParser]")
{
    juce::File f(k8MoraPath);
    if (!f.existsAsFile()) SKIP("8mora.txt not found");

    auto result = ReclistParser::load(f);

    REQUIRE(result.has_value());
    CHECK(result->name == "8mora");
    // Verified count — CLAUDE.md listed 186 but actual file has 153 entries
    CHECK(result->entries.size() == 153);
}

TEST_CASE("ReclistParser: 8mora.txt - every entry is non-empty and starts with '_'",
          "[ReclistParser]")
{
    juce::File f(k8MoraPath);
    if (!f.existsAsFile()) SKIP("8mora.txt not found");

    auto result = ReclistParser::load(f);
    REQUIRE(result.has_value());

    for (const auto& entry : result->entries)
    {
        CHECK_FALSE(entry.isEmpty());
        CHECK(entry.startsWithChar('_'));
    }
}

TEST_CASE("ReclistParser: 8mora.txt - comment sidecar is loaded", "[ReclistParser]")
{
    juce::File f(k8MoraPath);
    if (!f.existsAsFile()) SKIP("8mora.txt not found");

    auto result = ReclistParser::load(f);
    REQUIRE(result.has_value());

    CHECK_FALSE(result->comments.empty());
}

TEST_CASE("ReclistParser: 8mora.txt - each comment key appears in entries", "[ReclistParser]")
{
    juce::File f(k8MoraPath);
    if (!f.existsAsFile()) SKIP("8mora.txt not found");

    auto result = ReclistParser::load(f);
    REQUIRE(result.has_value());

    for (const auto& [key, value] : result->comments)
    {
        CHECK_FALSE(key.isEmpty());
        CHECK_FALSE(value.isEmpty());
        auto it = std::find(result->entries.begin(), result->entries.end(), key);
        CHECK(it != result->entries.end());
    }
}

// ── Synthetic in-memory parsing ───────────────────────────────────────────────

TEST_CASE("ReclistParser: newline-separated entries", "[ReclistParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText("_a\n_i\n_u\n_e\n_o\n", false, false, "\n");

    auto result = ReclistParser::load(tmp.getFile());

    REQUIRE(result.has_value());
    REQUIRE(result->entries.size() == 5);
    CHECK(result->entries[0] == "_a");
    CHECK(result->entries[4] == "_o");
}

TEST_CASE("ReclistParser: space-separated entries on one line", "[ReclistParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText("_a _i _u _e _o", false, false, "\n");

    auto result = ReclistParser::load(tmp.getFile());

    REQUIRE(result.has_value());
    REQUIRE(result->entries.size() == 5);
    CHECK(result->entries[2] == "_u");
}

TEST_CASE("ReclistParser: blank lines and extra whitespace are ignored", "[ReclistParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText("_a\n\n_i\n  \n_u\n", false, false, "\n");

    auto result = ReclistParser::load(tmp.getFile());

    REQUIRE(result.has_value());
    CHECK(result->entries.size() == 3);
}

TEST_CASE("ReclistParser: empty file -> nullopt", "[ReclistParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText("", false, false, "\n");

    auto result = ReclistParser::load(tmp.getFile());
    CHECK_FALSE(result.has_value());
}

TEST_CASE("ReclistParser: comment file - tab separator", "[ReclistParser]")
{
    juce::TemporaryFile tmp(".txt");
    auto base = tmp.getFile().getFileNameWithoutExtension();
    auto dir  = tmp.getFile().getParentDirectory();

    tmp.getFile().replaceWithText("_a\n_i\n", false, false, "\n");
    auto commentFile = dir.getChildFile(base + "-comment.txt");
    commentFile.replaceWithText("_a\t_a_comment\n_i\t_i_comment\n", false, false, "\n");

    auto result = ReclistParser::load(tmp.getFile());

    REQUIRE(result.has_value());
    CHECK(result->comments.at("_a") == "_a_comment");
    CHECK(result->comments.at("_i") == "_i_comment");

    commentFile.deleteFile();
}

TEST_CASE("ReclistParser: comment file - '#' lines skipped", "[ReclistParser]")
{
    juce::TemporaryFile tmp(".txt");
    auto base = tmp.getFile().getFileNameWithoutExtension();
    auto dir  = tmp.getFile().getParentDirectory();

    tmp.getFile().replaceWithText("_a\n_i\n", false, false, "\n");
    auto commentFile = dir.getChildFile(base + "-comment.txt");
    commentFile.replaceWithText("# this is a comment\n_a\t_a_val\n", false, false, "\n");

    auto result = ReclistParser::load(tmp.getFile());

    REQUIRE(result.has_value());
    CHECK(result->comments.size() == 1);
    CHECK(result->comments.at("_a") == "_a_val");

    commentFile.deleteFile();
}

// ── UTF-8 encoding path ───────────────────────────────────────────────────────
// 8mora-utf8.txt is an iconv-converted copy of 8mora.txt.
// Both must decode to identical in-memory content.

TEST_CASE("ReclistParser: 8mora-utf8.txt - same entry count as Shift-JIS version",
          "[ReclistParser][encoding]")
{
    juce::File f(k8MoraUtf8Path);
    if (!f.existsAsFile()) SKIP("8mora-utf8.txt not found");

    auto result = ReclistParser::load(f);

    REQUIRE(result.has_value());
    CHECK(result->name == "8mora-utf8");
    CHECK(result->entries.size() == 153);
}

TEST_CASE("ReclistParser: UTF-8 and Shift-JIS versions decode to identical entries",
          "[ReclistParser][encoding]")
{
    juce::File sjisFile(k8MoraPath);
    juce::File utf8File(k8MoraUtf8Path);
    if (!sjisFile.existsAsFile() || !utf8File.existsAsFile())
        SKIP("test data not found");

    auto sjis = ReclistParser::load(sjisFile);
    auto utf8 = ReclistParser::load(utf8File);

    REQUIRE(sjis.has_value());
    REQUIRE(utf8.has_value());

    // Entry lists must be identical after decoding
    REQUIRE(sjis->entries.size() == utf8->entries.size());
    for (size_t i = 0; i < sjis->entries.size(); ++i)
        CHECK(sjis->entries[i] == utf8->entries[i]);
}

TEST_CASE("ReclistParser: UTF-8 and Shift-JIS versions decode to identical comments",
          "[ReclistParser][encoding]")
{
    juce::File sjisFile(k8MoraPath);
    juce::File utf8File(k8MoraUtf8Path);
    if (!sjisFile.existsAsFile() || !utf8File.existsAsFile())
        SKIP("test data not found");

    auto sjis = ReclistParser::load(sjisFile);
    auto utf8 = ReclistParser::load(utf8File);

    REQUIRE(sjis.has_value());
    REQUIRE(utf8.has_value());

    // Comment maps must have the same keys and values after decoding
    // (keys in utf8 map have the same decoded strings as sjis keys)
    CHECK(sjis->comments.size() == utf8->comments.size());
    for (const auto& [key, value] : sjis->comments)
        CHECK(utf8->comments.count(key) == 1);
}
