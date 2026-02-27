#include "PlaybackControls.h"
#include "../PluginProcessor.h"

PlaybackControls::PlaybackControls(KiraNastroProcessor &processor)
    : processor(processor) {
  // Create buttons
  playButton = std::make_unique<juce::TextButton>("Play");
  stopButton = std::make_unique<juce::TextButton>("Stop");
  nextButton = std::make_unique<juce::TextButton>("Next");

  // Add and configure buttons
  addAndMakeVisible(playButton.get());
  addAndMakeVisible(stopButton.get());
  addAndMakeVisible(nextButton.get());

  playButton->addListener(this);
  stopButton->addListener(this);
  nextButton->addListener(this);

  playButton->setColour(juce::TextButton::buttonColourId,
                        juce::Colour(0xFF1A3FC7));
  playButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);

  stopButton->setColour(juce::TextButton::buttonColourId,
                        juce::Colour(0xFFE74C3C));
  stopButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);

  nextButton->setColour(juce::TextButton::buttonColourId,
                        juce::Colour(0xFF27AE60));
  nextButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);

  // Set the component's size last to avoid calling resized() before buttons are
  // ready
}

void PlaybackControls::paint(juce::Graphics &g) {
  juce::ignoreUnused(g);
  // Optional: paint a background for the controls
}

void PlaybackControls::resized() {
  if (playButton == nullptr || stopButton == nullptr || nextButton == nullptr)
    return;

  const int buttonWidth = 120;
  const int buttonHeight = 40;
  const int spacing = 16;
  const int totalWidth = (buttonWidth * 3) + (spacing * 2);

  int x = (getWidth() - totalWidth) / 2;
  int y = (getHeight() - buttonHeight) / 2;

  playButton->setBounds(x, y, buttonWidth, buttonHeight);
  x += buttonWidth + spacing;
  stopButton->setBounds(x, y, buttonWidth, buttonHeight);
  x += buttonWidth + spacing;
  nextButton->setBounds(x, y, buttonWidth, buttonHeight);
}

void PlaybackControls::buttonClicked(juce::Button *button) {
  if (button == playButton.get()) {
    processor.startBGM();
  } else if (button == stopButton.get()) {
    processor.stopBGM();
  } else if (button == nextButton.get()) {
    const double cycle = processor.getBGMLengthSeconds();
    const double currentPos = processor.projectPlayPositionSeconds.load();

    // Seek to the start of the next cycle
    // (This will also trigger currentEntryIndex advancement internally in
    // seekBGM)
    processor.seekBGM(std::floor(currentPos / cycle + 1.1) * cycle);
  }
}