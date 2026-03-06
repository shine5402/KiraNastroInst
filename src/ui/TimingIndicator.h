// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Pie chart + vertical fill bar widget showing BGM cycle progress and utterance stage.
// Total size: 48×32 (32px pie | 8px gap | 8px bar).
// The bar shows overall progress; the pie shows the current performance stage.
class TimingIndicator : public juce::Component
{
public:
    TimingIndicator() = default;
    ~TimingIndicator() override = default;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // Set the overall progress value [0.0, 1.0] — call from the editor's timer
    void setProgress(float newProgress)
    {
        m_progress = newProgress;
        repaint();
    }

    // Set utterance stage boundaries — call when BGM is loaded
    void setStageBoundaries(float utteranceStartFraction, float utteranceEndFraction)
    {
        m_utteranceStartFraction = utteranceStartFraction;
        m_utteranceEndFraction   = utteranceEndFraction;
        repaint();
    }

private:
    float m_progress               = 0.0f;
    float m_utteranceStartFraction = 0.333f;
    float m_utteranceEndFraction   = 0.667f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimingIndicator)
};
