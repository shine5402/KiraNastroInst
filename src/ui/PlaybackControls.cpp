#include "PlaybackControls.h"
#include "../PluginProcessor.h"

PlaybackControls::PlaybackControls(KiraNastroProcessor &processor)
    : processor(processor) {
  // Set the component's size first
  setSize(800, 60);

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

  // Set fixed positions for buttons
  playButton->setBounds(300, 10, 100, 40);
  stopButton->setBounds(410, 10, 100, 40);
  nextButton->setBounds(520, 10, 100, 40);
}

void PlaybackControls::paint(juce::Graphics &g) {
  // Optional: paint a background for the controls
}

void PlaybackControls::resized() {
  // Fixed positions, no need for resizing logic
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