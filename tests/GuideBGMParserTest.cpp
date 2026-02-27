#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "data/GuideBGMParser.h"

// Bundled test data — see tests/testdata/NOTES.md for license info
// Shift-JIS originals (marked binary in .gitattributes); UTF-8 copy alongside.
static constexpr const char* kJazzWavPath     = TEST_DATA_DIR "/Jazz-100-A.wav";
static constexpr const char* kJazzTxtPath     = TEST_DATA_DIR "/Jazz-100-A.txt";
static constexpr const char* kJazzTxtUtf8Path = TEST_DATA_DIR "/Jazz-100-A-utf8.txt";

// ── Error cases ───────────────────────────────────────────────────────────────

TEST_CASE("GuideBGMParser: non-existent timing file → nullopt", "[GuideBGMParser]")
{
    auto result = GuideBGMParser::loadFromTimingFile(juce::File("/does/not/exist.txt"));
    CHECK_FALSE(result.has_value());
}

TEST_CASE("GuideBGMParser: load() with non-existent wav → nullopt", "[GuideBGMParser]")
{
    auto result = GuideBGMParser::load(juce::File("/does/not/exist.wav"));
    CHECK_FALSE(result.has_value());
}

// ── Real Jazz-100-A.txt (6 nodes, msec unit) ─────────────────────────────────

TEST_CASE("GuideBGMParser: Jazz-100-A.txt parses 6 nodes", "[GuideBGMParser]")
{
    juce::File f(kJazzTxtPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.txt not found");

    auto result = GuideBGMParser::loadFromTimingFile(f);

    REQUIRE(result.has_value());
    CHECK(result->name == "Jazz-100-A");
    CHECK(result->nodes.size() == 6);
}

TEST_CASE("GuideBGMParser: Jazz-100-A.txt — times are positive and increasing",
          "[GuideBGMParser]")
{
    juce::File f(kJazzTxtPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.txt not found");

    auto result = GuideBGMParser::loadFromTimingFile(f);
    REQUIRE(result.has_value());

    double prev = -1.0;
    for (const auto& node : result->nodes)
    {
        CHECK(node.timeMs > prev);
        prev = node.timeMs;
    }
}

TEST_CASE("GuideBGMParser: Jazz-100-A.txt — node[1] is recording start", "[GuideBGMParser]")
{
    juce::File f(kJazzTxtPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.txt not found");

    auto result = GuideBGMParser::loadFromTimingFile(f);
    REQUIRE(result.has_value());
    REQUIRE(result->nodes.size() >= 2);

    const auto& node = result->nodes[1]; // row 2 in file
    CHECK(node.isRecordingStart == true);
    CHECK(node.isRecordingEnd   == false);
    CHECK(node.isSwitching      == false);
}

TEST_CASE("GuideBGMParser: Jazz-100-A.txt — node[4] is recording end", "[GuideBGMParser]")
{
    juce::File f(kJazzTxtPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.txt not found");

    auto result = GuideBGMParser::loadFromTimingFile(f);
    REQUIRE(result.has_value());
    REQUIRE(result->nodes.size() >= 5);

    const auto& node = result->nodes[4]; // row 5 in file
    CHECK(node.isRecordingStart == false);
    CHECK(node.isRecordingEnd   == true);
    CHECK(node.isSwitching      == false);
}

TEST_CASE("GuideBGMParser: Jazz-100-A.txt — node[5] switches and loops to node[0]",
          "[GuideBGMParser]")
{
    juce::File f(kJazzTxtPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.txt not found");

    auto result = GuideBGMParser::loadFromTimingFile(f);
    REQUIRE(result.has_value());
    REQUIRE(result->nodes.size() == 6);

    const auto& node = result->nodes[5]; // row 6 in file
    CHECK(node.isSwitching           == true);
    CHECK(node.repeatTargetNodeIndex == 0); // repeatTarget=1 in file → 0-indexed
}

TEST_CASE("GuideBGMParser: load() delegates to sibling .txt of .wav", "[GuideBGMParser]")
{
    juce::File f(kJazzWavPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.wav not found");

    auto result = GuideBGMParser::load(f);

    REQUIRE(result.has_value());
    CHECK(result->nodes.size() == 6);
}

// ── Synthetic in-memory parsing ───────────────────────────────────────────────

TEST_CASE("GuideBGMParser: rejects file with wrong/missing unit line", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText("beats\n1, 1.0, 0, 0, 0, 0\n", false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    CHECK_FALSE(result.has_value());
}

TEST_CASE("GuideBGMParser: msec unit — times stored as-is", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText(
        "msec\n"
        "1, 1000.0, 1, 0, 0, 0\n"
        "2, 2000.0, 0, 1, 0, 0\n",
        false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    REQUIRE(result.has_value());
    REQUIRE(result->nodes.size() == 2);
    CHECK(result->nodes[0].timeMs == Catch::Approx(1000.0));
    CHECK(result->nodes[1].timeMs == Catch::Approx(2000.0));
    CHECK(result->nodes[0].isRecordingStart == true);
    CHECK(result->nodes[1].isRecordingEnd   == true);
}

TEST_CASE("GuideBGMParser: sec unit — times multiplied by 1000", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText(
        "sec\n"
        "1, 1.5, 0, 0, 0, 0\n"
        "2, 3.0, 0, 0, 1, 1\n",
        false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    REQUIRE(result.has_value());
    REQUIRE(result->nodes.size() == 2);
    CHECK(result->nodes[0].timeMs == Catch::Approx(1500.0));
    CHECK(result->nodes[1].timeMs == Catch::Approx(3000.0));
}

TEST_CASE("GuideBGMParser: repeatTarget 0 in file → -1 (no repeat)", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText(
        "msec\n"
        "1, 100.0, 0, 0, 0, 0\n",
        false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    REQUIRE(result.has_value());
    CHECK(result->nodes[0].repeatTargetNodeIndex == -1);
}

TEST_CASE("GuideBGMParser: repeatTarget N in file → N-1 (0-indexed)", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText(
        "msec\n"
        "1, 100.0, 0, 0, 1, 1\n",
        false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    REQUIRE(result.has_value());
    CHECK(result->nodes[0].repeatTargetNodeIndex == 0);
}

TEST_CASE("GuideBGMParser: '#' comment lines are skipped", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText(
        "msec\n"
        "# this is a comment\n"
        "1, 500.0, 0, 0, 0, 0\n"
        "# another comment\n"
        "2, 1000.0, 0, 0, 0, 0\n",
        false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    REQUIRE(result.has_value());
    CHECK(result->nodes.size() == 2);
}

TEST_CASE("GuideBGMParser: trailing comment field is captured", "[GuideBGMParser]")
{
    juce::TemporaryFile tmp(".txt");
    tmp.getFile().replaceWithText(
        "msec\n"
        "1, 100.0, 0, 0, 0, 0, BGM start\n",
        false, false, "\n");

    auto result = GuideBGMParser::loadFromTimingFile(tmp.getFile());
    REQUIRE(result.has_value());
    CHECK(result->nodes[0].comment == "BGM start");
}

// ── UTF-8 encoding path ───────────────────────────────────────────────────────
// Jazz-100-A-utf8.txt is an iconv-converted copy of Jazz-100-A.txt.
// Both must decode to identical node data (including Japanese comment strings).

TEST_CASE("GuideBGMParser: Jazz-100-A-utf8.txt — same 6 nodes as Shift-JIS version",
          "[GuideBGMParser][encoding]")
{
    juce::File f(kJazzTxtUtf8Path);
    if (!f.existsAsFile()) SKIP("Jazz-100-A-utf8.txt not found");

    auto result = GuideBGMParser::loadFromTimingFile(f);

    REQUIRE(result.has_value());
    CHECK(result->nodes.size() == 6);
}

TEST_CASE("GuideBGMParser: UTF-8 and Shift-JIS versions decode to identical nodes",
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
    REQUIRE(sjis->nodes.size() == utf8->nodes.size());

    for (size_t i = 0; i < sjis->nodes.size(); ++i)
    {
        CHECK(sjis->nodes[i].timeMs               == Catch::Approx(utf8->nodes[i].timeMs));
        CHECK(sjis->nodes[i].isRecordingStart      == utf8->nodes[i].isRecordingStart);
        CHECK(sjis->nodes[i].isRecordingEnd        == utf8->nodes[i].isRecordingEnd);
        CHECK(sjis->nodes[i].isSwitching           == utf8->nodes[i].isSwitching);
        CHECK(sjis->nodes[i].repeatTargetNodeIndex == utf8->nodes[i].repeatTargetNodeIndex);
        CHECK(sjis->nodes[i].comment               == utf8->nodes[i].comment);
    }
}

TEST_CASE("GuideBGMParser: UTF-8 timing file — Japanese comment decodes correctly",
          "[GuideBGMParser][encoding]")
{
    juce::File f(kJazzTxtUtf8Path);
    if (!f.existsAsFile()) SKIP("Jazz-100-A-utf8.txt not found");

    auto result = GuideBGMParser::loadFromTimingFile(f);
    REQUIRE(result.has_value());
    REQUIRE(result->nodes.size() == 6);

    // Node 0 comment: "BGM再生" — verify non-empty and contains Japanese
    CHECK_FALSE(result->nodes[0].comment.isEmpty());
    // Node 5 comment: the long save-and-advance description
    CHECK_FALSE(result->nodes[5].comment.isEmpty());
    // Both must survive the round-trip identical to the Shift-JIS version
    // (covered by the "identical nodes" test above)
}
