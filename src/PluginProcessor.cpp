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

  // --- Timing scheduler logic ---
  // We process the buffer in slices to handle timing nodes (especially loops)
  // seamlessly within a single block.

  std::vector<TimingNode> nodes;
  {
    juce::ScopedLock sl(dataLock);
    if (bgmData.has_value())
      nodes = bgmData->nodes;
  }
  if (nodes.empty())
    return;

  const int numSamplesInBlock = buffer.getNumSamples();
  int samplesRendered = 0;
  const int numNodes = static_cast<int>(nodes.size());

  while (samplesRendered < numSamplesInBlock) {
    const int64_t currentBgmSamples = bgmPlayer.getCurrentPositionSamples();
    const double currentBgmPosMs =
        (static_cast<double>(currentBgmSamples) / currentSampleRate) * 1000.0;

    // Determine how many samples to render in this slice
    int samplesToRenderInSlice = numSamplesInBlock - samplesRendered;

    // Check if we will cross any nodes in this slice
    bool nodeTriggered = false;
    if (currentNodeIndex < numNodes) {
      const auto &node = nodes[currentNodeIndex];
      const double samplesToReachNode =
          ((node.timeMs - currentBgmPosMs) / 1000.0) * currentSampleRate;

      if (samplesToReachNode <= 0.0) {
        // We are already at or past this node
        nodeTriggered = true;
        samplesToRenderInSlice = 0;
      } else if (samplesToReachNode <
                 static_cast<double>(samplesToRenderInSlice)) {
        // We will hit this node within the current slice. Ceil ensures we
        // render at least 1 sample to push us over the threshold.
        samplesToRenderInSlice =
            static_cast<int>(std::ceil(samplesToReachNode));
        if (samplesToRenderInSlice < 0)
          samplesToRenderInSlice = 0;
      }
    }

    // Render the slice
    if (samplesToRenderInSlice > 0) {
      bgmPlayer.renderNextBlock(buffer, samplesRendered,
                                samplesToRenderInSlice);
      samplesRendered += samplesToRenderInSlice;

      // Update project position
      projectPlayPositionSeconds.store(
          projectPlayPositionSeconds.load(std::memory_order_relaxed) +
              static_cast<double>(samplesToRenderInSlice) / currentSampleRate,
          std::memory_order_relaxed);
    }

    // Process nodes that were reached
    if (currentNodeIndex < numNodes) {
      const auto currentBgmSamplesAfter = bgmPlayer.getCurrentPositionSamples();
      const double currentBgmMsAfter =
          (static_cast<double>(currentBgmSamplesAfter) / currentSampleRate) *
          1000.0;

      // Use a small epsilon to catch nodes exactly at the boundary
      while (currentNodeIndex < numNodes &&
             currentBgmMsAfter >= nodes[currentNodeIndex].timeMs - 0.001) {
        const auto &node = nodes[currentNodeIndex];

        // Entry switching (advance reclist)
        if (node.isSwitching) {
          const int current = currentEntryIndex.load();
          const int total = totalEntries.load();
          if (current < total - 1) {
            currentEntryIndex.store(current + 1);
          } else {
            // End of reclist — stop
            bgmPlayer.stop();
            isBGMPlayingFlag = false;
            return;
          }
        }

        // Repeat / loop: seek BGM back
        if (node.repeatTargetNodeIndex >= 0 &&
            node.repeatTargetNodeIndex < numNodes) {
          const double targetTimeMs = nodes[node.repeatTargetNodeIndex].timeMs;
          const int64_t targetSamples =
              static_cast<int64_t>((targetTimeMs / 1000.0) * currentSampleRate);

          bgmPlayer.seekToSample(targetSamples);
          currentNodeIndex = node.repeatTargetNodeIndex;
          // After a loop, we re-evaluate from the new position in the next
          // slice of the SAME block.
          break;
        }

        ++currentNodeIndex;
      }
    }

    // Safety: if we didn't render anything and didn't trigger any nodes,
    // we must advance to avoid an infinite loop.
    if (samplesToRenderInSlice == 0 && !nodeTriggered) {
      break;
    }

    // If we exhausted all nodes without a repeat loop, stop at the very end
    if (currentNodeIndex >= numNodes && samplesRendered < numSamplesInBlock) {
      // Just fill the rest with silence (already cleared)
      break;
    }
  }

  // Update loop progress for UI (once per block is fine)
  if (numNodes >= 2) {
    const double curMs =
        (static_cast<double>(bgmPlayer.getCurrentPositionSamples()) /
         currentSampleRate) *
        1000.0;
    const double cycleStartTime = nodes.front().timeMs;
    const double cycleEndTime = nodes.back().timeMs;
    const double cycleDuration = cycleEndTime - cycleStartTime;
    if (cycleDuration > 0.0) {
      float progress =
          static_cast<float>((curMs - cycleStartTime) / cycleDuration);
      bgmLoopProgress.store(std::clamp(progress, 0.0f, 1.0f),
                            std::memory_order_relaxed);
    }
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
    currentNodeIndex = 0;
  }

  // Seek immediately to the start of the loop cycle to bypass any intro
  if (wavOk) {
    seekBGM(0.0);
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

std::optional<GuideBGMData> KiraNastroProcessor::getGuideBGMData() const {
  juce::ScopedLock sl(dataLock);
  return bgmData;
}

bool KiraNastroProcessor::isBGMLoaded() const { return bgmPlayer.isLoaded(); }

void KiraNastroProcessor::startBGM() {
  // Just start playing from current position
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
  if (bgmPlayer.isLoaded()) {
    double loopStartMs = 0.0;
    double loopEndMs = static_cast<double>(bgmPlayer.getTotalSamples()) /
                       bgmPlayer.getSampleRate() * 1000.0;

    {
      juce::ScopedLock sl(dataLock);
      if (bgmData.has_value() && !bgmData->nodes.empty()) {
        loopEndMs = bgmData->nodes.back().timeMs;
        int targetIdx = bgmData->nodes.back().repeatTargetNodeIndex;
        if (targetIdx >= 0 && targetIdx < (int)bgmData->nodes.size()) {
          loopStartMs = bgmData->nodes[targetIdx].timeMs;
        }
      }
    }
    return (loopEndMs - loopStartMs) / 1000.0;
  }
  return 0.0;
}

void KiraNastroProcessor::seekBGM(double seconds) {
  if (!bgmPlayer.isLoaded())
    return;

  // Identify loop segment from nodes
  double loopStartTimeMs = 0.0;
  double loopEndTimeMs = getBGMLengthSeconds() * 1000.0;
  double loopDurationMs = loopEndTimeMs;

  {
    juce::ScopedLock sl(dataLock);
    if (bgmData.has_value() && !bgmData->nodes.empty()) {
      // Find the first node that is a repeat target, or just use front/back
      loopEndTimeMs = bgmData->nodes.back().timeMs;

      // If the last node repeats to something, use that as start
      int targetIdx = bgmData->nodes.back().repeatTargetNodeIndex;
      if (targetIdx >= 0 && targetIdx < (int)bgmData->nodes.size()) {
        loopStartTimeMs = bgmData->nodes[targetIdx].timeMs;
      }

      loopDurationMs = loopEndTimeMs - loopStartTimeMs;
    }
  }

  double sessionMs = seconds * 1000.0;
  double targetBgmMs = sessionMs;
  int entryIndex = 0;

  if (loopDurationMs > 0.0) {
    if (sessionMs < 0.0)
      sessionMs = 0.0;

    // Concentrate looping: the session timeline is mapped entirely
    // to the loop body, skipping the intro entirely on all cycles.
    entryIndex = static_cast<int>(sessionMs / loopDurationMs);
    targetBgmMs = loopStartTimeMs + std::fmod(sessionMs, loopDurationMs);
  }

  // Set the calculated entry index (clamped to reclist size)
  const int total = totalEntries.load();
  if (total > 0) {
    currentEntryIndex.store(std::clamp(entryIndex, 0, total - 1));
  }

  const int64_t targetSamples =
      static_cast<int64_t>((targetBgmMs / 1000.0) * currentSampleRate);
  bgmPlayer.seekToSample(targetSamples);
  projectPlayPositionSeconds.store(seconds, std::memory_order_relaxed);

  // Reset the node index to the appropriate position for the new BGM time
  juce::ScopedLock sl(dataLock);
  if (bgmData.has_value()) {
    currentNodeIndex = 0;
    for (int i = 0; i < static_cast<int>(bgmData->nodes.size()); ++i) {
      if (bgmData->nodes[i].timeMs < targetBgmMs - 0.001)
        currentNodeIndex = i + 1;
      else
        break;
    }
  }
}

//==============================================================================
// Plugin entry point — required by JUCE
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new KiraNastroProcessor();
}
