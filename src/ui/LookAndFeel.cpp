#include "LookAndFeel.h"

const juce::Colour KiraNastroLookAndFeel::primaryBlue { primaryBlueValue };
const juce::Colour KiraNastroLookAndFeel::navyBar     { navyBarValue };
const juce::Colour KiraNastroLookAndFeel::surface     { surfaceValue };
const juce::Colour KiraNastroLookAndFeel::background  { backgroundValue };

KiraNastroLookAndFeel::KiraNastroLookAndFeel()
{
    // Apply MD3 palette to standard JUCE colour IDs
    setColour(juce::ResizableWindow::backgroundColourId, background);
    setColour(juce::TextButton::buttonColourId,          primaryBlue);
    setColour(juce::TextButton::textColourOffId,         surface);
    setColour(juce::Label::textColourId,                 primaryBlue);
    setColour(juce::ScrollBar::thumbColourId,            primaryBlue);
}

void KiraNastroLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                  juce::Button& button,
                                                  const juce::Colour& backgroundColour,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    // Delegate to base for now — Phase 4 will add MD3 styling
    LookAndFeel_V4::drawButtonBackground(g, button, backgroundColour,
                                         shouldDrawButtonAsHighlighted,
                                         shouldDrawButtonAsDown);
}

void KiraNastroLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    // Delegate to base for now — Phase 4 will add custom typography
    LookAndFeel_V4::drawLabel(g, label);
}
