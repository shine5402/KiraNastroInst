#include "PluginProcessor.h"
#include "PluginEditor.h"

KiraNastroProcessor::KiraNastroProcessor()
    : AudioProcessor(BusesProperties().withOutput(
          "Output", juce::AudioChannelSet::stereo(), true)) {}

KiraNastroProcessor::~KiraNastroProcessor() {}

//==============================================================================
void KiraNastroProcessor::prepareToPlay(double sampleRate,
                                        int samplesPerBlock) {
  currentSampleRate = sampleRate;
  currentBlockSize = samplesPerBlock;
  bgmPlayer.prepareToPlay(sampleRate, samplesPerBlock);
}

void KiraNastroProcessor::releaseResources() {
  bgmPlayer.stop();
  bgmPlayer.unload();
}

bool KiraNastroProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  // Stereo output only; no audio input required (VSTi)
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;
  return true;
}

void KiraNastroProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                       juce::MidiBuffer & /*midiMessages*/) {
  juce::ScopedNoDenormals noDenormals;

  buffer.clear();

  if (!bgmPlayer.isLoaded() || !isBGMPlayingFlag)
    return;

  // --- Simple block-loop playback ---
  // The BGM "block" is defined by [bgmBlockStartMs, bgmBlockEndMs).
  // We render audio within this range. When playback reaches blockEnd,
  // we seek back to blockStart and advance the reclist entry.

  const double blockStartMs = bgmBlockStartMs;
  const double blockEndMs   = bgmBlockEndMs;

  if (blockEndMs <= blockStartMs)
    return; // invalid bounds — nothing to play

  // BGMPlayer positions are in file-sample units, so convert ms using the
  // file's sample rate, NOT the host sample rate.
  const double fileSampleRate = static_cast<double>(bgmPlayer.getSampleRate());
  if (fileSampleRate <= 0.0)
    return;

  const int64_t blockStartSamples =
      static_cast<int64_t>((blockStartMs / 1000.0) * fileSampleRate);
  const int64_t blockEndSamples =
      static_cast<int64_t>((blockEndMs / 1000.0) * fileSampleRate);

  const int numSamplesInBlock = buffer.getNumSamples();
  int samplesRendered = 0;

  while (samplesRendered < numSamplesInBlock) {
    const int64_t currentPos = bgmPlayer.getCurrentPositionSamples();

    // If we're already at or past the block end, loop back
    if (currentPos >= blockEndSamples) {
      // Advance reclist entry
      const int current = currentEntryIndex.load();
      const int total = totalEntries.load();
      if (current < total - 1) {
        currentEntryIndex.store(current + 1);
      } else {
        // End of reclist — stop playback
        bgmPlayer.stop();
        isBGMPlayingFlag = false;
        return;
      }

      bgmPlayer.seekToSample(blockStartSamples);
      continue; // re-evaluate position from the top
    }

    // How many host samples remain until the block boundary?
    // samplesUntilEnd is in file-sample space; convert to host samples.
    const int64_t fileSamplesUntilEnd = blockEndSamples - currentPos;
    const double playbackRatio = fileSampleRate / currentSampleRate;
    const int64_t hostSamplesUntilEnd =
        static_cast<int64_t>(std::ceil(
            static_cast<double>(fileSamplesUntilEnd) / playbackRatio));
    const int remaining = numSamplesInBlock - samplesRendered;
    const int samplesToRender =
        static_cast<int>(std::min(static_cast<int64_t>(remaining),
                                  hostSamplesUntilEnd));

    if (samplesToRender <= 0)
      break; // safety

    bgmPlayer.renderNextBlock(buffer, samplesRendered, samplesToRender);
    samplesRendered += samplesToRender;

    // Update project position
    projectPlayPositionSeconds.store(
        projectPlayPositionSeconds.load(std::memory_order_relaxed) +
            static_cast<double>(samplesToRender) / currentSampleRate,
        std::memory_order_relaxed);
  }

  // Update loop progress for UI
  const double cycleDuration = blockEndMs - blockStartMs;
  if (cycleDuration > 0.0) {
    const double curMs =
        (static_cast<double>(bgmPlayer.getCurrentPositionSamples()) /
         fileSampleRate) *
        1000.0;
    float progress =
        static_cast<float>((curMs - blockStartMs) / cycleDuration);
    bgmLoopProgress.store(std::clamp(progress, 0.0f, 1.0f),
                          std::memory_order_relaxed);
  }
}

//==============================================================================
juce::AudioProcessorEditor *KiraNastroProcessor::createEditor() {
  return new KiraNastroEditor(*this);
}

//==============================================================================
void KiraNastroProcessor::getStateInformation(
    juce::MemoryBlock & /*destData*/) {
  // TODO Phase 6: save reclist path, BGM path, currentEntryIndex, settings
}

void KiraNastroProcessor::setStateInformation(const void * /*data*/,
                                              int /*sizeInBytes*/) {
  // TODO Phase 6: restore session state
}

//==============================================================================
// Data loading

bool KiraNastroProcessor::loadReclist(const juce::File &reclistFile) {
  auto result = ReclistParser::load(reclistFile);

  juce::ScopedLock sl(dataLock);
  reclistData = result;

  if (result.has_value()) {
    currentEntryIndex.store(0);
    totalEntries.store(static_cast<int>(result->entries.size()));
    return true;
  }

  totalEntries.store(0);
  return false;
}

bool KiraNastroProcessor::loadGuideBGM(const juce::File &wavFile) {
  auto timingResult = GuideBGMParser::load(wavFile);
  bool wavOk = bgmPlayer.loadFile(wavFile);

  {
    juce::ScopedLock sl(dataLock);
    bgmData = timingResult;
  }

  // Compute BGM block boundaries from timing nodes.
  // Block start = first node's time.
  // Block end   = time of the node that has a repeatTargetNodeIndex set.
  bgmBlockStartMs = 0.0;
  bgmBlockEndMs = 0.0;
  recordingStartOffsetMs = 0.0;
  recordingWindowDurationMs = 0.0;

  if (timingResult.has_value() && !timingResult->nodes.empty()) {
    bgmBlockStartMs = timingResult->nodes.front().timeMs;

    // Find the node with repeatTargetNodeIndex >= 0 (typically the last node)
    for (auto it = timingResult->nodes.rbegin();
         it != timingResult->nodes.rend(); ++it) {
      if (it->repeatTargetNodeIndex >= 0) {
        bgmBlockEndMs = it->timeMs;
        break;
      }
    }

    // Fallback: if no repeat node found, use the last node's time
    if (bgmBlockEndMs <= bgmBlockStartMs) {
      bgmBlockEndMs = timingResult->nodes.back().timeMs;
    }

    // Find recording window offsets
    double recordingStartAbsMs = bgmBlockStartMs; // fallback: recording starts at block start
    for (const auto& node : timingResult->nodes) {
      if (node.isRecordingStart)
        recordingStartAbsMs = node.timeMs;
      if (node.isRecordingEnd && node.timeMs > recordingStartAbsMs) {
        recordingStartOffsetMs   = recordingStartAbsMs - bgmBlockStartMs;
        recordingWindowDurationMs = node.timeMs - recordingStartAbsMs;
        break;
      }
    }
  }

  // Seek to block start so playback begins at the right position
  if (wavOk && bgmBlockEndMs > bgmBlockStartMs) {
    const double fileSR = static_cast<double>(bgmPlayer.getSampleRate());
    const int64_t startSamples =
        static_cast<int64_t>((bgmBlockStartMs / 1000.0) * fileSR);
    bgmPlayer.seekToSample(startSamples);
  }

  return wavOk;
}

std::optional<ReclistData> KiraNastroProcessor::getReclistData() const {
  juce::ScopedLock sl(dataLock);
  return reclistData;
}

KiraNastroProcessor::EntryInfo
KiraNastroProcessor::getCurrentEntryInfo() const {
  juce::ScopedLock sl(dataLock);
  EntryInfo info;
  info.index = currentEntryIndex.load();
  info.total = totalEntries.load();

  if (reclistData.has_value() && info.index >= 0 &&
      static_cast<size_t>(info.index) < reclistData->entries.size()) {
    info.name = reclistData->entries[static_cast<size_t>(info.index)];
    if (reclistData->comments.count(info.name))
      info.comment = reclistData->comments.at(info.name);
  }
  return info;
}

KiraNastroProcessor::EntryInfo
KiraNastroProcessor::getNextEntryInfo() const {
  juce::ScopedLock sl(dataLock);
  EntryInfo info;
  info.index = currentEntryIndex.load() + 1;
  info.total = totalEntries.load();

  if (reclistData.has_value() && info.index >= 0 &&
      static_cast<size_t>(info.index) < reclistData->entries.size()) {
    info.name = reclistData->entries[static_cast<size_t>(info.index)];
    if (reclistData->comments.count(info.name))
      info.comment = reclistData->comments.at(info.name);
  }
  return info;
}

std::optional<GuideBGMData> KiraNastroProcessor::getGuideBGMData() const {
  juce::ScopedLock sl(dataLock);
  return bgmData;
}

bool KiraNastroProcessor::isBGMLoaded() const { return bgmPlayer.isLoaded(); }

void KiraNastroProcessor::startBGM() {
  // Play from the current position (set by loadGuideBGM or seekBGM).
  // Do NOT reset to blockStart here — that would undo any user seek.
  bgmPlayer.play();
  isBGMPlayingFlag = true;
}

void KiraNastroProcessor::stopBGM() {
  bgmPlayer.stop();
  isBGMPlayingFlag = false;
  // If we want a true 'Stop' behavior like OREMO, we should reset.
  // But let's keep it as Pause for now to satisfy the user's resume need.
  // Actually, let's keep it as is.
}

bool KiraNastroProcessor::isBGMPlaying() const { return isBGMPlayingFlag; }

double KiraNastroProcessor::getBGMLengthSeconds() const {
  if (bgmPlayer.isLoaded() && bgmBlockEndMs > bgmBlockStartMs) {
    return (bgmBlockEndMs - bgmBlockStartMs) / 1000.0;
  }
  return 0.0;
}

void KiraNastroProcessor::seekBGM(double seconds) {
  if (!bgmPlayer.isLoaded())
    return;

  const double blockDurationMs = bgmBlockEndMs - bgmBlockStartMs;
  if (blockDurationMs <= 0.0)
    return;

  double sessionMs = std::max(seconds * 1000.0, 0.0);

  // Map session time to the loop: each block cycle corresponds to one entry.
  int entryIndex = static_cast<int>(sessionMs / blockDurationMs);
  double offsetWithinBlock = std::fmod(sessionMs, blockDurationMs);
  double targetBgmMs = bgmBlockStartMs + offsetWithinBlock;

  // Clamp entry index to reclist bounds
  const int total = totalEntries.load();
  if (total > 0) {
    currentEntryIndex.store(std::clamp(entryIndex, 0, total - 1));
  }

  const double fileSR = static_cast<double>(bgmPlayer.getSampleRate());
  const int64_t targetSamples =
      static_cast<int64_t>((targetBgmMs / 1000.0) * fileSR);
  bgmPlayer.seekToSample(targetSamples);
  projectPlayPositionSeconds.store(seconds, std::memory_order_relaxed);

  // Update loop progress so the timing indicator reflects the seek immediately,
  // even when paused (processBlock only runs while playing).
  const double cycleDuration = bgmBlockEndMs - bgmBlockStartMs;
  if (cycleDuration > 0.0) {
    const float progress =
        static_cast<float>((targetBgmMs - bgmBlockStartMs) / cycleDuration);
    bgmLoopProgress.store(std::clamp(progress, 0.0f, 1.0f),
                          std::memory_order_relaxed);
  }
}

KiraNastroProcessor::DescExportParams
KiraNastroProcessor::getDescExportParams() const {
  DescExportParams p;
  juce::ScopedLock sl(dataLock);
  if (reclistData.has_value())
    p.entryNames = reclistData->entries;
  p.blockDurationSec        = (bgmBlockEndMs - bgmBlockStartMs) / 1000.0;
  p.recordingStartOffsetSec = recordingStartOffsetMs / 1000.0;
  p.recordingWindowDurationSec = recordingWindowDurationMs / 1000.0;
  p.sampleRate = bgmPlayer.getSampleRate() > 0
                     ? static_cast<double>(bgmPlayer.getSampleRate())
                     : 44100.0;
  return p;
}

//==============================================================================
// Plugin entry point — required by JUCE
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new KiraNastroProcessor();
}
