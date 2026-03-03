// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class KiraNastroProcessor;

class PlaybackControls : public juce::Component, public juce::Button::Listener
{
public:
    PlaybackControls(KiraNastroProcessor &processor);
    ~PlaybackControls() override = default;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void buttonClicked(juce::Button *button) override;

private:
    KiraNastroProcessor &m_processor;

    std::unique_ptr<juce::TextButton> m_playButton;
    std::unique_ptr<juce::TextButton> m_stopButton;
    std::unique_ptr<juce::TextButton> m_nextButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackControls)
};