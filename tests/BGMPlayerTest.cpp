// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audio/BGMPlayer.h"
#include <catch2/catch_test_macros.hpp>

// Bundled test data — see tests/testdata/NOTES.md for license info
static constexpr const char *kJazzWavPath  = TEST_DATA_DIR "/Jazz-100-A.wav";
static constexpr const char *kFlacPath     = TEST_DATA_DIR "/test_stereo.flac";
static constexpr const char *kOggPath      = TEST_DATA_DIR "/test_stereo.ogg";
static constexpr const char *kMp3Path      = TEST_DATA_DIR "/test_stereo.mp3";
static constexpr const char *kOpusPath     = TEST_DATA_DIR "/test_stereo.opus";
static constexpr const char *kAiffPath     = TEST_DATA_DIR "/test_stereo.aiff";
static constexpr const char *kMonoFlacPath = TEST_DATA_DIR "/test_mono.flac";
// AAC and ALAC are platform-dependent (CoreAudio on macOS, WindowsMediaAudio
// on Windows) — tests are skipped automatically when the files aren't readable.
static constexpr const char *kAacPath      = TEST_DATA_DIR "/test_stereo_aac.m4a";
static constexpr const char *kAlacPath     = TEST_DATA_DIR "/test_stereo_alac.m4a";

// ── Initial state
// ─────────────────────────────────────────────────────────────

TEST_CASE("BGMPlayer: default-constructed state", "[BGMPlayer]") {
  BGMPlayer player;
  CHECK(player.isLoaded() == false);
  CHECK(player.getSampleRate() == 0);
  CHECK(player.getNumChannels() == 0);
  CHECK(player.getTotalSamples() == 0);
}

// ── unload() on empty player doesn't crash
// ────────────────────────────────────

TEST_CASE("BGMPlayer: unload() on fresh player is a no-op", "[BGMPlayer]") {
  BGMPlayer player;
  player.unload(); // should not crash
  CHECK(player.isLoaded() == false);
}

// ── loadFile() error cases
// ────────────────────────────────────────────────────

TEST_CASE("BGMPlayer: non-existent file -> loadFile returns false",
          "[BGMPlayer]") {
  BGMPlayer player;
  bool ok = player.loadFile(juce::File("/does/not/exist.wav"));
  CHECK(ok == false);
  CHECK(player.isLoaded() == false);
}

// ── loadFile() with real WAV
// ──────────────────────────────────────────────────

TEST_CASE("BGMPlayer: loads Jazz-100-A.wav successfully", "[BGMPlayer]") {
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  bool ok = player.loadFile(f);

  REQUIRE(ok == true);
  CHECK(player.isLoaded() == true);
  CHECK(player.getSampleRate() > 0);
  CHECK(player.getNumChannels() > 0);
  CHECK(player.getTotalSamples() > 0);
}

TEST_CASE("BGMPlayer: loaded WAV has stereo channels", "[BGMPlayer]") {
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);
  CHECK(player.getNumChannels() == 2);
}

TEST_CASE("BGMPlayer: sample count consistent with sample rate x duration",
          "[BGMPlayer]") {
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);

  // Jazz-100-A BGM is ~20 s - sanity-check it's between 5 s and 60 s
  double durationSeconds = static_cast<double>(player.getTotalSamples()) /
                           static_cast<double>(player.getSampleRate());
  CHECK(durationSeconds > 5.0);
  CHECK(durationSeconds < 60.0);
}

// ── unload() resets state
// ─────────────────────────────────────────────────────

TEST_CASE("BGMPlayer: unload() resets to clean state", "[BGMPlayer]") {
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);
  REQUIRE(player.isLoaded());

  player.unload();
  CHECK(player.isLoaded() == false);
  CHECK(player.getSampleRate() == 0);
  CHECK(player.getNumChannels() == 0);
  CHECK(player.getTotalSamples() == 0);
}

TEST_CASE("BGMPlayer: reload after unload works", "[BGMPlayer]") {
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);
  player.unload();
  bool ok = player.loadFile(f);
  CHECK(ok == true);
  CHECK(player.isLoaded() == true);
}

// ── Phase 3: Playback tests ──────────────────────────────────────────────────

TEST_CASE("BGMPlayer: play/stop toggle", "[BGMPlayer]") {
  BGMPlayer player;
  CHECK(player.isPlaying() == false);

  player.play();
  CHECK(player.isPlaying() == true);

  player.stop();
  CHECK(player.isPlaying() == false);
}

TEST_CASE("BGMPlayer: initial position is zero", "[BGMPlayer]") {
  BGMPlayer player;
  CHECK(player.getCurrentPositionSamples() == 0);
}

TEST_CASE("BGMPlayer: seekToSample sets position", "[BGMPlayer]") {
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);

  player.seekToSample(1000);
  CHECK(player.getCurrentPositionSamples() == 1000);

  // Clamps to total samples
  player.seekToSample(player.getTotalSamples() + 999);
  CHECK(player.getCurrentPositionSamples() == player.getTotalSamples());

  // Clamps to zero
  player.seekToSample(-100);
  CHECK(player.getCurrentPositionSamples() == 0);
}

TEST_CASE("BGMPlayer: renderNextBlock advances position", "[BGMPlayer]") {
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);
  player.prepareToPlay(player.getSampleRate(), 512);
  player.seekToSample(0);
  player.play();

  juce::AudioBuffer<float> output(2, 512);
  output.clear();
  player.renderNextBlock(output, 0, 512);

  // Position should have advanced by approximately 512 samples
  // (exactly 512 when host and file sample rates match)
  CHECK(player.getCurrentPositionSamples() > 0);
  CHECK(player.getCurrentPositionSamples() <= 512);
}

TEST_CASE("BGMPlayer: renderNextBlock produces non-silent output",
          "[BGMPlayer]") {
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);
  player.prepareToPlay(player.getSampleRate(), 512);
  // Seek past any leading silence in the BGM
  player.seekToSample(player.getSampleRate()); // 1 second in
  player.play();

  juce::AudioBuffer<float> output(2, 512);
  output.clear();
  player.renderNextBlock(output, 0, 512);

  // At least one sample should be non-zero
  float magnitude = output.getMagnitude(0, 512);
  CHECK(magnitude > 0.0f);
}

TEST_CASE("BGMPlayer: renderNextBlock does nothing when stopped",
          "[BGMPlayer]") {
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);
  player.prepareToPlay(player.getSampleRate(), 512);
  player.seekToSample(0);
  // Don't call play()

  juce::AudioBuffer<float> output(2, 512);
  output.clear();
  player.renderNextBlock(output, 0, 512);

  CHECK(player.getCurrentPositionSamples() == 0);
  CHECK(output.getMagnitude(0, 512) == 0.0f);
}

TEST_CASE("BGMPlayer: loop-by-seek pattern works (core looping feature)",
          "[BGMPlayer]") {
  // This tests the key behavior: the timing scheduler calls seekToSample()
  // to loop the BGM back to an earlier position.  After seeking, the BGM
  // must continue playing from the new position.
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);
  player.prepareToPlay(player.getSampleRate(), 256);
  player.play();

  // Play forward for 256 samples
  juce::AudioBuffer<float> output(2, 256);
  output.clear();
  player.renderNextBlock(output, 0, 256);
  auto posAfterFirstBlock = player.getCurrentPositionSamples();
  CHECK(posAfterFirstBlock > 0);

  // Seek back to sample 0 (simulating a timing node repeat)
  player.seekToSample(0);
  CHECK(player.getCurrentPositionSamples() == 0);
  CHECK(player.isPlaying() == true); // must still be playing

  // Play forward again — should produce audio from the start
  output.clear();
  player.renderNextBlock(output, 0, 256);
  CHECK(player.getCurrentPositionSamples() > 0);
}

TEST_CASE("BGMPlayer: does NOT auto-stop at end of file", "[BGMPlayer]") {
  // The timing scheduler controls looping — BGMPlayer must not
  // unilaterally stop when it reaches the last sample.
  juce::File f(kJazzWavPath);
  if (!f.existsAsFile())
    SKIP("Jazz-100-A.wav not found");

  BGMPlayer player;
  player.loadFile(f);
  player.prepareToPlay(player.getSampleRate(), 256);

  // Position the player near the end of the file
  const int64_t nearEnd = player.getTotalSamples() - 100;
  player.seekToSample(nearEnd);
  player.play();

  // Render a block that will cross the end boundary
  juce::AudioBuffer<float> output(2, 256);
  output.clear();
  player.renderNextBlock(output, 0, 256);

  // The player must still be "playing" — it did NOT call stop()
  CHECK(player.isPlaying() == true);
}

// ── Multi-format loading ─────────────────────────────────────────────────────
// Each test file is a 2-second stereo excerpt of Jazz-100-A.wav encoded with
// ffmpeg.  Tests verify that BGMPlayer can load and render audio for every
// format that registerBasicFormats() + OpusAudioFormat cover.

// Shared helper: load a file, verify basic properties, render a block, check
// the output is non-silent.  Returns false (causing SKIP) if the file doesn't
// exist on this machine.
static void checkFormatLoad(const char *path, int expectedChannels,
                             int minSampleRate)
{
    juce::File f(path);
    if (!f.existsAsFile())
        SKIP(juce::String(path).fromLastOccurrenceOf("/", false, false) + " not found");

    BGMPlayer player;
    REQUIRE(player.loadFile(f) == true);
    CHECK(player.isLoaded() == true);
    CHECK(player.getSampleRate() >= minSampleRate);
    CHECK(player.getNumChannels() == expectedChannels);
    CHECK(player.getTotalSamples() > 0);

    // Duration sanity: the clips are 2 s — accept 1–10 s to accommodate
    // encoder/decoder padding (MP3 has ~1152-sample frame padding, Opus has
    // a pre-roll).
    const double dur = static_cast<double>(player.getTotalSamples()) /
                       static_cast<double>(player.getSampleRate());
    CHECK(dur > 1.0);
    CHECK(dur < 10.0);

    // Render a block 1 second in and confirm we get non-silent output
    player.prepareToPlay(player.getSampleRate(), 512);
    player.seekToSample(static_cast<int64_t>(player.getSampleRate())); // 1 s in
    player.play();

    juce::AudioBuffer<float> buf(expectedChannels, 512);
    buf.clear();
    player.renderNextBlock(buf, 0, 512);

    CHECK(buf.getMagnitude(0, 512) > 0.0f);
}

TEST_CASE("BGMPlayer: loads FLAC (stereo)", "[BGMPlayer][formats]") {
    checkFormatLoad(kFlacPath, 2, 44100);
}

TEST_CASE("BGMPlayer: loads OGG Vorbis (stereo)", "[BGMPlayer][formats]") {
    checkFormatLoad(kOggPath, 2, 44100);
}

TEST_CASE("BGMPlayer: loads MP3 (stereo)", "[BGMPlayer][formats]") {
    checkFormatLoad(kMp3Path, 2, 44100);
}

TEST_CASE("BGMPlayer: loads Opus (stereo)", "[BGMPlayer][formats]") {
    // Opus always decodes to 48 kHz regardless of source sample rate
    checkFormatLoad(kOpusPath, 2, 48000);
}

TEST_CASE("BGMPlayer: loads AIFF (stereo)", "[BGMPlayer][formats]") {
    checkFormatLoad(kAiffPath, 2, 44100);
}

TEST_CASE("BGMPlayer: loads mono FLAC", "[BGMPlayer][formats]") {
    checkFormatLoad(kMonoFlacPath, 1, 44100);
}

// Helper for platform-codec formats (AAC, ALAC): if loadFile() returns false
// the codec is simply unavailable on this platform — skip rather than fail.
static void checkPlatformFormatLoad(const char *path, int expectedChannels,
                                    int minSampleRate)
{
    juce::File f(path);
    if (!f.existsAsFile())
        SKIP(juce::String(path).fromLastOccurrenceOf("/", false, false) + " not found");

    BGMPlayer player;
    if (!player.loadFile(f))
        SKIP(juce::String(path).fromLastOccurrenceOf("/", false, false) +
             ": codec not available on this platform");

    CHECK(player.isLoaded() == true);
    CHECK(player.getSampleRate() >= minSampleRate);
    CHECK(player.getNumChannels() == expectedChannels);
    CHECK(player.getTotalSamples() > 0);

    const double dur = static_cast<double>(player.getTotalSamples()) /
                       static_cast<double>(player.getSampleRate());
    CHECK(dur > 1.0);
    CHECK(dur < 10.0);

    player.prepareToPlay(player.getSampleRate(), 512);
    player.seekToSample(static_cast<int64_t>(player.getSampleRate()));
    player.play();

    juce::AudioBuffer<float> buf(expectedChannels, 512);
    buf.clear();
    player.renderNextBlock(buf, 0, 512);
    CHECK(buf.getMagnitude(0, 512) > 0.0f);
}

TEST_CASE("BGMPlayer: loads AAC in M4A container (macOS/Windows only)",
          "[BGMPlayer][formats]") {
    checkPlatformFormatLoad(kAacPath, 2, 44100);
}

TEST_CASE("BGMPlayer: loads ALAC in M4A container (macOS/Windows only)",
          "[BGMPlayer][formats]") {
    checkPlatformFormatLoad(kAlacPath, 2, 44100);
}

TEST_CASE("BGMPlayer: getWildcardForAllFormats includes expected extensions",
          "[BGMPlayer][formats]") {
    BGMPlayer player;
    const juce::String wildcards = player.getWildcardForAllFormats();

    CHECK(wildcards.containsIgnoreCase("*.wav"));
    CHECK(wildcards.containsIgnoreCase("*.flac"));
    CHECK(wildcards.containsIgnoreCase("*.ogg"));
    CHECK(wildcards.containsIgnoreCase("*.mp3"));
    CHECK(wildcards.containsIgnoreCase("*.opus"));
    CHECK(wildcards.containsIgnoreCase("*.aif"));
}
