#include "TimingIndicator.h"

void TimingIndicator::paint(juce::Graphics &g) {
  auto bounds = getLocalBounds().toFloat().reduced(2.0f);

  // Background circle
  g.setColour(juce::Colours::darkgrey.withAlpha(0.1f));
  g.fillEllipse(bounds);

  // Border
  g.setColour(juce::Colours::grey.withAlpha(0.5f));
  g.drawEllipse(bounds, 1.5f);

  // Pie chart arc (filled)
  // Starting from the top (-PI/2) and moving clockwise
  juce::Path p;
  p.addPieSegment(bounds.getX(), bounds.getY(), bounds.getWidth(),
                  bounds.getHeight(), -juce::MathConstants<float>::halfPi,
                  -juce::MathConstants<float>::halfPi +
                      (progress * juce::MathConstants<float>::twoPi),
                  0.0f);

  g.setColour(juce::Colour(0xFF1A3FC7).withMultipliedBrightness(1.5f));
  g.fillPath(p);
}

void TimingIndicator::resized() {}
