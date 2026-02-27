#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "data/GuideBGMParser.h"

// Bundled test data — see tests/testdata/NOTES.md for license info
static constexpr const char* kJazzWavPath = TEST_DATA_DIR "/Jazz-100-A.wav";
static constexpr const char* kJazzTxtPath = TEST_DATA_DIR "/Jazz-100-A.txt";

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
