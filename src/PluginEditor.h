#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/LookAndFeel.h"

class KiraNastroEditor : public juce::AudioProcessorEditor,
                         public juce::Timer
{
public:
    explicit KiraNastroEditor(KiraNastroProcessor&);
    ~KiraNastroEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void timerCallback() override;

private:
    KiraNastroProcessor& audioProcessor;
    KiraNastroLookAndFeel lookAndFeel;

    // Cached state for change detection
    int lastEntryIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroEditor)
};
