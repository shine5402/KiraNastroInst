#include "TimingIndicator.h"

void TimingIndicator::paint(juce::Graphics& g)
{
    // TODO Phase 4: draw pie chart arc based on `progress`
    g.setColour(juce::Colours::grey);
    g.drawEllipse(getLocalBounds().toFloat().reduced(2.0f), 1.0f);
}

void TimingIndicator::resized()
{
    // No child components
}
