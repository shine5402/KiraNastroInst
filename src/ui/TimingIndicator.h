#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Phase 4: Pie chart widget showing the current position within the BGM cycle.
// Reads a normalised [0,1] progress value and repaints on a timer tick.
class TimingIndicator : public juce::Component
{
public:
    TimingIndicator() = default;
    ~TimingIndicator() override = default;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // Set the progress value [0.0, 1.0] — call from the editor's timer
    void setProgress(float newProgress)
    {
        m_progress = newProgress;
        repaint();
    }

    // TODO Phase 4: draw filled arc, border, cue markers

private:
    float m_progress = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimingIndicator)
};
