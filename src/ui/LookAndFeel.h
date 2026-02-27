#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class KiraNastroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // MD3-inspired color palette
    static constexpr juce::uint32 primaryBlueValue = 0xFF1A3FC7;
    static constexpr juce::uint32 navyBarValue     = 0xFF1A3978;
    static constexpr juce::uint32 surfaceValue     = 0xFFFFFFFF;
    static constexpr juce::uint32 backgroundValue  = 0xFFF5F7FB;

    static const juce::Colour primaryBlue;
    static const juce::Colour navyBar;
    static const juce::Colour surface;
    static const juce::Colour background;

    KiraNastroLookAndFeel();
    ~KiraNastroLookAndFeel() override = default;

    // Override stubs — will be fleshed out in Phase 4
    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroLookAndFeel)
};
