// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class KiraNastroProcessor;

class PlaybackControls : public juce::Component, public juce::Timer
{
public:
    PlaybackControls(KiraNastroProcessor &processor);
    ~PlaybackControls() override = default;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void timerCallback() override;

private:
    KiraNastroProcessor &m_processor;

    std::unique_ptr<juce::DrawableButton> m_playStopButton;
    std::unique_ptr<juce::DrawableButton> m_prevButton;
    std::unique_ptr<juce::DrawableButton> m_nextButton;

    // Cached drawables for play/stop toggle
    std::unique_ptr<juce::Drawable> m_playNormal, m_playHover, m_playPressed;
    std::unique_ptr<juce::Drawable> m_stopNormal, m_stopHover, m_stopPressed;

    bool m_wasPlaying = false;

    void updatePlayStopIcon();
    void drawStateLayer(juce::Graphics &g, juce::DrawableButton *btn);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackControls)
};
