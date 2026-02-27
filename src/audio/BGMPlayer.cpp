#include "BGMPlayer.h"

BGMPlayer::BGMPlayer()
{
    formatManager.registerBasicFormats();
}

bool BGMPlayer::loadFile(const juce::File& wavFile)
{
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
    auto numSamples  = static_cast<int>(reader->lengthInSamples);

    audioBuffer.setSize(numChannels, numSamples);
    reader->read(&audioBuffer, 0, numSamples, 0, true, true);

    loadedSampleRate = static_cast<int>(reader->sampleRate);
    loaded = true;

    return true;
}

void BGMPlayer::unload()
{
    audioBuffer.setSize(0, 0);
    loadedSampleRate = 0;
    loaded = false;
}

int BGMPlayer::getNumChannels() const noexcept
{
    return audioBuffer.getNumChannels();
}

int BGMPlayer::getTotalSamples() const noexcept
{
    return audioBuffer.getNumSamples();
}
