// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "ui/LookAndFeel.h"
#include "ui/PlaybackControls.h"
#include "ui/TimingIndicator.h"

class KiraNastroEditor : public juce::AudioProcessorEditor, public juce::Timer, public juce::Slider::Listener
{
public:
    explicit KiraNastroEditor(KiraNastroProcessor &);
    ~KiraNastroEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    //==============================================================================
    void timerCallback() override;
    void sliderValueChanged(juce::Slider *) override;

private:
    KiraNastroProcessor &m_audioProcessor;
    KiraNastroLookAndFeel m_lookAndFeel;

    // Cached state for change detection
    int m_lastEntryIndex = -1;
    juce::String m_currentEntryName;
    juce::String m_currentEntryComment;
    juce::String m_nextEntryName;
    juce::String m_nextEntryComment;

    // File choosers
    std::unique_ptr<juce::FileChooser> m_reclistChooser;
    std::unique_ptr<juce::FileChooser> m_bgmChooser;
    std::unique_ptr<juce::FileChooser> m_descChooser;

    // Hamburger menu button (always present)
    std::unique_ptr<juce::DrawableButton> m_menuButton;

    // Brand logo loaded from SVG
    std::unique_ptr<juce::Drawable> m_logoDrawable;

    // Chip icons (Material Symbols, tinted onSecondaryContainer)
    std::unique_ptr<juce::Drawable> m_nextIcon;
    std::unique_ptr<juce::Drawable> m_progressIcon;

    // Timing pie chart (always present)
    std::unique_ptr<TimingIndicator> m_timingIndicator;

    // Standalone-only debug controls (null in plugin mode; created at runtime
    // based on wrapperType — compile-time guards don't work in JUCE shared code)
    std::unique_ptr<PlaybackControls> m_playbackControls;
    std::unique_ptr<juce::Slider> m_progressSlider;

    void showMenu();
    void reloadChipIcons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroEditor)
};
