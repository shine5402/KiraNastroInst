#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Phase 4: Main plugin UI — shows current reclist entry, comment, next preview,
// progress counter, timing indicator, and bottom bar.
class MainComponent : public juce::Component
{
public:
    MainComponent() = default;
    ~MainComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // TODO Phase 4: add child components (labels, TimingIndicator, bottom bar)

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
