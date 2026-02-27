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
// Plugin entry point — required by JUCE
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KiraNastroProcessor();
}
