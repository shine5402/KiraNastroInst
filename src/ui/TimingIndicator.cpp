#include "TimingIndicator.h"

void TimingIndicator::paint(juce::Graphics &g) {
  auto bounds = getLocalBounds().toFloat();

  // Empty track (MD3 surfaceVariant — no stroke)
  g.setColour(juce::Colour(0xFFE2E1EC));
  g.fillEllipse(bounds);

  // Pie chart arc (MD3 primary)
  juce::Path p;
  p.addPieSegment(bounds.getX(), bounds.getY(), bounds.getWidth(),
                  bounds.getHeight(), -juce::MathConstants<float>::halfPi,
                  -juce::MathConstants<float>::halfPi +
                      (progress * juce::MathConstants<float>::twoPi),
                  0.0f);
  g.setColour(juce::Colour(0xFF4E5B92));  // MD3 primary
  g.fillPath(p);
}

void TimingIndicator::resized() {}
