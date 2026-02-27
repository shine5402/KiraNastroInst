#include "PluginProcessor.h"
#include "PluginEditor.h"

KiraNastroProcessor::KiraNastroProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

KiraNastroProcessor::~KiraNastroProcessor() {}

//==============================================================================
void KiraNastroProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    // TODO Phase 3: prepare BGMPlayer with new sample rate/block size
}

void KiraNastroProcessor::releaseResources()
{
    // TODO Phase 3: release BGMPlayer resources
}

bool KiraNastroProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Stereo output only; no audio input required (VSTi)
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void KiraNastroProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear output — BGM rendering will be added in Phase 3
    buffer.clear();

    // TODO Phase 3: render BGM audio into buffer based on timing scheduler
}

//==============================================================================
juce::AudioProcessorEditor* KiraNastroProcessor::createEditor()
{
    return new KiraNastroEditor(*this);
}

//==============================================================================
void KiraNastroProcessor::getStateInformation(juce::MemoryBlock& /*destData*/)
{
    // TODO Phase 6: save reclist path, BGM path, currentEntryIndex, settings
}

void KiraNastroProcessor::setStateInformation(const void* /*data*/, int /*sizeInBytes*/)
{
    // TODO Phase 6: restore session state
}

//==============================================================================
// Data loading

bool KiraNastroProcessor::loadReclist(const juce::File& reclistFile)
{
    auto result = ReclistParser::load(reclistFile);

    juce::ScopedLock sl(dataLock);
    reclistData = result;

    if (result.has_value())
    {
        currentEntryIndex.store(0);
        totalEntries.store(static_cast<int>(result->entries.size()));
        return true;
    }

    totalEntries.store(0);
    return false;
}

bool KiraNastroProcessor::loadGuideBGM(const juce::File& wavFile)
{
    auto timingResult = GuideBGMParser::load(wavFile);
    bool wavOk        = bgmPlayer.loadFile(wavFile);

    {
        juce::ScopedLock sl(dataLock);
        bgmData = timingResult;
    }

    return wavOk;
}

std::optional<ReclistData> KiraNastroProcessor::getReclistData() const
{
    juce::ScopedLock sl(dataLock);
    return reclistData;
}

std::optional<GuideBGMData> KiraNastroProcessor::getGuideBGMData() const
{
    juce::ScopedLock sl(dataLock);
    return bgmData;
}

bool KiraNastroProcessor::isBGMLoaded() const
{
    return bgmPlayer.isLoaded();
}

//==============================================================================
// Plugin entry point — required by JUCE
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KiraNastroProcessor();
}
