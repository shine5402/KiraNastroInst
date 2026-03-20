// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "OpusAudioFormat.h"

#include <opusfile.h>

#include <algorithm>
#include <vector>

namespace {

//==============================================================================
// libopusfile I/O callbacks bridging a JUCE InputStream

static int opusReadCb(void* stream, unsigned char* ptr, int nbytes)
{
    return static_cast<juce::InputStream*>(stream)->read(ptr, nbytes);
}

static int opusSeekCb(void* stream, opus_int64 offset, int whence)
{
    auto* is = static_cast<juce::InputStream*>(stream);
    juce::int64 newPos;
    if (whence == SEEK_SET)
        newPos = static_cast<juce::int64>(offset);
    else if (whence == SEEK_CUR)
        newPos = is->getPosition() + static_cast<juce::int64>(offset);
    else if (whence == SEEK_END)
        newPos = is->getTotalLength() + static_cast<juce::int64>(offset);
    else
        return -1;

    return is->setPosition(newPos) ? 0 : -1;
}

static opus_int64 opusTellCb(void* stream)
{
    return static_cast<opus_int64>(static_cast<juce::InputStream*>(stream)->getPosition());
}

static int opusCloseCb(void* /*stream*/)
{
    return 0; // Stream lifetime is managed by JUCE
}

//==============================================================================
static constexpr int kDecodeChunkFrames = 4096;

class OpusReader final : public juce::AudioFormatReader
{
public:
    OpusReader(juce::InputStream* stream, bool deleteStreamIfOpeningFails)
        : juce::AudioFormatReader(stream, "Opus")
    {
        const OpusFileCallbacks cbs = { opusReadCb, opusSeekCb, opusTellCb, opusCloseCb };
        int error = 0;
        m_of = op_open_callbacks(stream, &cbs, nullptr, 0, &error);

        if (m_of == nullptr || error != 0) {
            // Prevent base class destructor from deleting the stream if the
            // caller wants to keep ownership on failure.
            if (!deleteStreamIfOpeningFails)
                input = nullptr;
            return;
        }

        sampleRate            = 48000; // Opus always decodes to 48 kHz
        usesFloatingPointData = true;
        numChannels           = static_cast<unsigned int>(op_channel_count(m_of, -1));
        lengthInSamples       = static_cast<juce::int64>(op_pcm_total(m_of, -1));

        // Seek back to the start after probing
        op_pcm_seek(m_of, 0);
    }

    ~OpusReader() override
    {
        if (m_of != nullptr)
            op_free(m_of);
        // Base class destructor deletes 'input' (the stream).
    }

    bool isValid() const noexcept { return m_of != nullptr; }

    bool readSamples(int* const* destSamples, int numDestChannels,
                     int startOffsetInDestBuffer, juce::int64 startSampleInFile,
                     int numSamples) override
    {
        if (m_of == nullptr)
            return false;

        // Seek if we're not at the right position
        if (startSampleInFile != m_currentPos) {
            if (op_pcm_seek(m_of, static_cast<ogg_int64_t>(startSampleInFile)) != 0)
                return false;
            m_currentPos      = startSampleInFile;
            m_reservoirFrames = 0;
            m_reservoirPos    = 0;
        }

        // Zero-fill the destination buffers
        for (int ch = 0; ch < numDestChannels; ++ch) {
            if (destSamples[ch] != nullptr) {
                juce::FloatVectorOperations::clear(
                    reinterpret_cast<float*>(destSamples[ch]) + startOffsetInDestBuffer,
                    numSamples);
            }
        }

        int written = 0;
        while (written < numSamples) {
            // Drain reservoir first
            const int avail = m_reservoirFrames - m_reservoirPos;
            if (avail > 0) {
                const int toCopy = std::min(avail, numSamples - written);
                const int srcChannels = static_cast<int>(numChannels);

                for (int s = 0; s < toCopy; ++s) {
                    for (int ch = 0; ch < numDestChannels; ++ch) {
                        if (destSamples[ch] != nullptr) {
                            auto* dst = reinterpret_cast<float*>(destSamples[ch]);
                            const int srcCh = (ch < srcChannels) ? ch : 0;
                            dst[startOffsetInDestBuffer + written + s] =
                                m_reservoir[static_cast<size_t>((m_reservoirPos + s) * srcChannels + srcCh)];
                        }
                    }
                }

                m_reservoirPos += toCopy;
                written        += toCopy;

                if (m_reservoirPos >= m_reservoirFrames) {
                    m_reservoirFrames = 0;
                    m_reservoirPos    = 0;
                }
                continue;
            }

            // Decode the next chunk from opusfile into the reservoir
            const int bufFloats = kDecodeChunkFrames * static_cast<int>(numChannels);
            m_reservoir.resize(static_cast<size_t>(bufFloats));
            int li = 0;
            const int decoded = op_read_float(m_of, m_reservoir.data(), bufFloats, &li);

            if (decoded <= 0)
                break; // EOF or decoding error

            m_reservoirFrames = decoded;
            m_reservoirPos    = 0;
        }

        m_currentPos += static_cast<juce::int64>(written);
        return true;
    }

private:
    OggOpusFile*       m_of              = nullptr;
    juce::int64        m_currentPos      = 0;
    std::vector<float> m_reservoir;
    int                m_reservoirFrames = 0;
    int                m_reservoirPos    = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpusReader)
};

} // anonymous namespace

//==============================================================================
OpusAudioFormat::OpusAudioFormat()
    : juce::AudioFormat("Opus audio file", ".opus .ogg")
{
}

juce::Array<int> OpusAudioFormat::getPossibleSampleRates() { return {48000}; }
juce::Array<int> OpusAudioFormat::getPossibleBitDepths()   { return {32}; }
bool OpusAudioFormat::canDoStereo()  { return true; }
bool OpusAudioFormat::canDoMono()    { return true; }
bool OpusAudioFormat::isCompressed() { return true; }

juce::AudioFormatReader* OpusAudioFormat::createReaderFor(juce::InputStream* sourceStream,
                                                          bool deleteStreamIfOpeningFails)
{
    if (sourceStream == nullptr)
        return nullptr;

    auto reader = std::make_unique<OpusReader>(sourceStream, deleteStreamIfOpeningFails);
    if (reader->isValid())
        return reader.release();

    return nullptr;
    // unique_ptr destructor runs OpusReader dtor → base class dtor deletes 'input'
    // (= sourceStream) if deleteStreamIfOpeningFails was true, or nullptr if false.
}

std::unique_ptr<juce::AudioFormatWriter> OpusAudioFormat::createWriterFor(
    std::unique_ptr<juce::OutputStream>& /*streamToWriteTo*/,
    const juce::AudioFormatWriterOptions& /*options*/)
{
    return nullptr; // Write support not implemented
}
