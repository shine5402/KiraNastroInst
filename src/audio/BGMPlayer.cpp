#include "BGMPlayer.h"

BGMPlayer::BGMPlayer() { formatManager.registerBasicFormats(); }

bool BGMPlayer::loadFile(const juce::File &wavFile) {
  unload();

  std::unique_ptr<juce::AudioFormatReader> reader(
      formatManager.createReaderFor(wavFile));

  if (reader == nullptr)
    return false;

  // Guard against unreasonably large files (> 5 minutes at 48 kHz)
  const int64_t maxSamples = static_cast<int64_t>(48000) * 60 * 5;
  if (reader->lengthInSamples > maxSamples)
    return false;

  auto numChannels = static_cast<int>(reader->numChannels);
  auto numSamples = static_cast<int>(reader->lengthInSamples);

  audioBuffer.setSize(numChannels, numSamples);
  reader->read(&audioBuffer, 0, numSamples, 0, true, true);

  loadedSampleRate = static_cast<int>(reader->sampleRate);
  loaded = true;

  // Recompute playback ratio in case prepareToPlay was called before loading
  if (hostSampleRate > 0.0 && loadedSampleRate > 0)
    playbackRatio = static_cast<double>(loadedSampleRate) / hostSampleRate;

  return true;
}

void BGMPlayer::unload() {
  audioBuffer.setSize(0, 0);
  loadedSampleRate = 0;
  loaded = false;
}

int BGMPlayer::getNumChannels() const noexcept {
  return audioBuffer.getNumChannels();
}

int BGMPlayer::getTotalSamples() const noexcept {
  return audioBuffer.getNumSamples();
}

void BGMPlayer::prepareToPlay(double sampleRate, int /*blockSize*/) {
  hostSampleRate = sampleRate;

  if (loadedSampleRate > 0)
    playbackRatio = static_cast<double>(loadedSampleRate) / sampleRate;
  else
    playbackRatio = 1.0;
}

void BGMPlayer::renderNextBlock(juce::AudioBuffer<float> &buffer,
                                int startSample, int numSamples) {
  if (!loaded || !isPlayingFlag.load(std::memory_order_relaxed))
    return;

  auto currentPos = currentPositionSamples.load(std::memory_order_relaxed);
  const auto totalSamples = static_cast<int64_t>(audioBuffer.getNumSamples());

  for (int sample = 0; sample < numSamples; ++sample) {
    const auto bufferPos = startSample + sample;

    // Produce audio only if within buffer
    if (currentPos < totalSamples) {
      for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch) {
        if (ch < buffer.getNumChannels()) {
          buffer.addSample(
              ch, bufferPos,
              audioBuffer.getSample(ch, static_cast<int>(currentPos)));
        }
      }
    }

    // ALWAYS advance position with fractional accuracy, even past EOF
    positionFraction += playbackRatio;
    currentPos += static_cast<int64_t>(positionFraction);
    positionFraction -= static_cast<int64_t>(positionFraction);
  }

  currentPositionSamples.store(currentPos, std::memory_order_relaxed);
}

void BGMPlayer::seekToSample(int64_t pos) {
  const auto totalSamples = static_cast<int64_t>(audioBuffer.getNumSamples());
  currentPositionSamples.store(std::clamp(pos, int64_t(0), totalSamples),
                               std::memory_order_relaxed);
  // Reset position fraction when seeking
  positionFraction = 0.0;
}

void BGMPlayer::play() { isPlayingFlag.store(true, std::memory_order_relaxed); }

void BGMPlayer::stop() {
  isPlayingFlag.store(false, std::memory_order_relaxed);
}

bool BGMPlayer::isPlaying() const noexcept {
  return isPlayingFlag.load(std::memory_order_relaxed);
}

int64_t BGMPlayer::getCurrentPositionSamples() const noexcept {
  return currentPositionSamples.load(std::memory_order_relaxed);
}
