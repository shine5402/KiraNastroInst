#include "TimingIndicator.h"

#include "LookAndFeel.h"

void TimingIndicator::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();

    // Get live colors from LookAndFeel
    if (auto *laf = dynamic_cast<KiraNastroLookAndFeel *>(&getLookAndFeel())) {
        // Empty track (MD3 surfaceVariant — no stroke)
        g.setColour(laf->surfaceVariant());
        g.fillEllipse(bounds);

        // Pie chart arc (MD3 primary)
        juce::Path p;
        p.addPieSegment(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                        -juce::MathConstants<float>::halfPi,
                        -juce::MathConstants<float>::halfPi + (m_progress * juce::MathConstants<float>::twoPi), 0.0f);
        g.setColour(laf->primary());
        g.fillPath(p);
    }
    else {
        // Fallback to hardcoded literals (shouldn't happen)
        g.setColour(juce::Colour(0xFFE2E1EC)); // empty track
        g.fillEllipse(bounds);

        juce::Path p;
        p.addPieSegment(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                        -juce::MathConstants<float>::halfPi,
                        -juce::MathConstants<float>::halfPi + (m_progress * juce::MathConstants<float>::twoPi), 0.0f);
        g.setColour(juce::Colour(0xFF4E5B92)); // arc fill
        g.fillPath(p);
    }
}

void TimingIndicator::resized()
{
}
