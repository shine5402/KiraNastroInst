#include <catch2/catch_test_macros.hpp>
#include "audio/BGMPlayer.h"

// Bundled test data — see tests/testdata/NOTES.md for license info
static constexpr const char* kJazzWavPath = TEST_DATA_DIR "/Jazz-100-A.wav";

// ── Initial state ─────────────────────────────────────────────────────────────

TEST_CASE("BGMPlayer: default-constructed state", "[BGMPlayer]")
{
    BGMPlayer player;
    CHECK(player.isLoaded()        == false);
    CHECK(player.getSampleRate()   == 0);
    CHECK(player.getNumChannels()  == 0);
    CHECK(player.getTotalSamples() == 0);
}

// ── unload() on empty player doesn't crash ────────────────────────────────────

TEST_CASE("BGMPlayer: unload() on fresh player is a no-op", "[BGMPlayer]")
{
    BGMPlayer player;
    player.unload(); // should not crash
    CHECK(player.isLoaded() == false);
}

// ── loadFile() error cases ────────────────────────────────────────────────────

TEST_CASE("BGMPlayer: non-existent file → loadFile returns false", "[BGMPlayer]")
{
    BGMPlayer player;
    bool ok = player.loadFile(juce::File("/does/not/exist.wav"));
    CHECK(ok == false);
    CHECK(player.isLoaded() == false);
}

// ── loadFile() with real WAV ──────────────────────────────────────────────────

TEST_CASE("BGMPlayer: loads Jazz-100-A.wav successfully", "[BGMPlayer]")
{
    juce::File f(kJazzWavPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.wav not found");

    BGMPlayer player;
    bool ok = player.loadFile(f);

    REQUIRE(ok == true);
    CHECK(player.isLoaded()        == true);
    CHECK(player.getSampleRate()   > 0);
    CHECK(player.getNumChannels()  > 0);
    CHECK(player.getTotalSamples() > 0);
}

TEST_CASE("BGMPlayer: loaded WAV has stereo channels", "[BGMPlayer]")
{
    juce::File f(kJazzWavPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.wav not found");

    BGMPlayer player;
    player.loadFile(f);
    CHECK(player.getNumChannels() == 2);
}

TEST_CASE("BGMPlayer: sample count consistent with sample rate × duration", "[BGMPlayer]")
{
    juce::File f(kJazzWavPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.wav not found");

    BGMPlayer player;
    player.loadFile(f);

    // Jazz-100-A BGM is ~20 s — sanity-check it's between 5 s and 60 s
    double durationSeconds = static_cast<double>(player.getTotalSamples())
                           / static_cast<double>(player.getSampleRate());
    CHECK(durationSeconds > 5.0);
    CHECK(durationSeconds < 60.0);
}

// ── unload() resets state ─────────────────────────────────────────────────────

TEST_CASE("BGMPlayer: unload() resets to clean state", "[BGMPlayer]")
{
    juce::File f(kJazzWavPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.wav not found");

    BGMPlayer player;
    player.loadFile(f);
    REQUIRE(player.isLoaded());

    player.unload();
    CHECK(player.isLoaded()        == false);
    CHECK(player.getSampleRate()   == 0);
    CHECK(player.getNumChannels()  == 0);
    CHECK(player.getTotalSamples() == 0);
}

TEST_CASE("BGMPlayer: reload after unload works", "[BGMPlayer]")
{
    juce::File f(kJazzWavPath);
    if (!f.existsAsFile()) SKIP("Jazz-100-A.wav not found");

    BGMPlayer player;
    player.loadFile(f);
    player.unload();
    bool ok = player.loadFile(f);
    CHECK(ok == true);
    CHECK(player.isLoaded() == true);
}
