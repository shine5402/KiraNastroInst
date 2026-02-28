#include "audio/BGMPlayer.h"
#include <catch2/catch_test_macros.hpp>

// Bundled test data — see tests/testdata/NOTES.md for license info
static constexpr const char *kJazzWavPath = TEST_DATA_DIR "/Jazz-100-A.wav";

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
