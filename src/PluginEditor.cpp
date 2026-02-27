#include "PluginEditor.h"
#include <cmath>

#ifdef JUCE_STANDALONE_APPLICATION
#include "ui/PlaybackControls.h"
#endif

KiraNastroEditor::KiraNastroEditor(KiraNastroProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setLookAndFeel(&lookAndFeel);

  // Create configuration UI
  loadReclistButton = std::make_unique<juce::TextButton>("Load Reclist");
  loadBGMButton = std::make_unique<juce::TextButton>("Load BGM");

  addAndMakeVisible(loadReclistButton.get());
  addAndMakeVisible(loadBGMButton.get());

  DBG("Adding listeners to buttons");
  loadReclistButton->addListener(this);
  loadBGMButton->addListener(this);
  DBG("Listeners added successfully");

  loadReclistButton->setColour(juce::TextButton::buttonColourId,
                               juce::Colour(0xFF1A3FC7));
  loadReclistButton->setColour(juce::TextButton::textColourOnId,
                               juce::Colours::white);

  loadBGMButton->setColour(juce::TextButton::buttonColourId,
                           juce::Colour(0xFF1A3FC7));
  loadBGMButton->setColour(juce::TextButton::textColourOnId,
                           juce::Colours::white);

// Create standalone-specific controls
#ifdef JUCE_STANDALONE_APPLICATION
  playbackControls = std::make_unique<PlaybackControls>(audioProcessor);
  addAndMakeVisible(playbackControls.get());

  progressSlider = std::make_unique<juce::Slider>();
  addAndMakeVisible(progressSlider.get());
  progressSlider->addListener(this);
  progressSlider->setRange(0.0, 600.0, 0.1); // Default 10 minutes max
  progressSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  progressSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
  progressSlider->setTextValueSuffix(" s");

  timingIndicator = std::make_unique<TimingIndicator>();
  addAndMakeVisible(timingIndicator.get());
#endif

  setSize(800, 320);
  startTimerHz(30);
}

KiraNastroEditor::~KiraNastroEditor() {
  stopTimer();
  setLookAndFeel(nullptr);
}

//==============================================================================
void KiraNastroEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(0xFFF5F7FB));

  // Main Card Area
  auto area = getLocalBounds().reduced(20);
#ifdef JUCE_STANDALONE_APPLICATION
  area.removeFromBottom(60); // Space for controls
#endif
  auto cardArea = area.removeFromTop(140);

  g.setColour(juce::Colours::white);
  g.fillRoundedRectangle(cardArea.toFloat(), 12.0f);

  g.setColour(juce::Colour(0xFF1A3FC7).withAlpha(0.1f));
  g.drawRoundedRectangle(cardArea.toFloat(), 12.0f, 2.0f);

  // Text in card
  auto textArea = cardArea.reduced(20);
  textArea.removeFromRight(100); // Space for TimingIndicator

  // Comment (Romaji)
  g.setColour(juce::Colours::grey);
  g.setFont(20.0f);
  g.drawFittedText(currentEntryComment, textArea.removeFromTop(30),
                   juce::Justification::left, 1);

  // Entry Name (Large Japanese)
  g.setColour(juce::Colour(0xFF1A3FC7));
  g.setFont(48.0f);
  g.drawFittedText(currentEntryName, textArea, juce::Justification::left, 1);

  // Total progress indicator at bottom
  const int total = audioProcessor.totalEntries.load();
  g.setFont(16.0f);
  g.setColour(juce::Colours::darkgrey);
  g.drawFittedText("Entry: " + juce::String(lastEntryIndex + 1) + " / " +
                       juce::String(total),
                   getLocalBounds().withTrimmedBottom(80),
                   juce::Justification::centredBottom, 1);
}

void KiraNastroEditor::resized() {
  // Configuration buttons
  loadReclistButton->setBounds(20, 20, 150, 40);
  loadBGMButton->setBounds(180, 20, 150, 40);

#ifdef JUCE_STANDALONE_APPLICATION
  auto area = getLocalBounds().reduced(20);
  auto cardArea = area.removeFromTop(140);
  if (timingIndicator)
    timingIndicator->setBounds(cardArea.removeFromRight(100).reduced(10));

  // Progress slider
  progressSlider->setBounds(20, 180, getWidth() - 40, 40);

  // Playback controls
  playbackControls->setBounds(0, getHeight() - 60, getWidth(), 60);
#endif
}

//==============================================================================
void KiraNastroEditor::timerCallback() {
  auto info = audioProcessor.getCurrentEntryInfo();
  if (info.index != lastEntryIndex || info.name != currentEntryName) {
    lastEntryIndex = info.index;
    currentEntryName = info.name;
    currentEntryComment = info.comment;
    repaint();
  }

#ifdef JUCE_STANDALONE_APPLICATION
  if (timingIndicator) {
    timingIndicator->setProgress(
        audioProcessor.bgmLoopProgress.load(std::memory_order_relaxed));
  }
  // Update progress slider based on project playback position
  if (progressSlider && audioProcessor.isBGMLoaded()) {
    const double currentPos = audioProcessor.projectPlayPositionSeconds.load();
    progressSlider->setValue(currentPos, juce::dontSendNotification);

    // Update slider range to be 10x BGM length as per design intent
    const double bgmLength = audioProcessor.getBGMLengthSeconds();
    const double projectLength = bgmLength * 10.0;
    const double currentMax = progressSlider->getMaximum();
    if (bgmLength > 0 && std::abs(currentMax - projectLength) > 0.1) {
      progressSlider->setRange(0.0, projectLength, 0.1);
    }
  }
#endif
}

void KiraNastroEditor::buttonClicked(juce::Button *button) {
  if (button == loadReclistButton.get()) {
    DBG("Load Reclist button clicked");
    // Open file chooser for reclist files
    reclistChooser = std::make_unique<juce::FileChooser>("Select Reclist File",
                                                         juce::File(), "*.txt");
    reclistChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                    juce::FileBrowserComponent::canSelectFiles,
                                [this](const juce::FileChooser &f) {
                                  auto result = f.getResult();
                                  DBG("Reclist file chosen: " +
                                      result.getFullPathName());
                                  if (result.exists()) {
                                    DBG("Reclist file exists, loading...");
                                    audioProcessor.loadReclist(result);
                                  } else {
                                    DBG("Reclist file does not exist");
                                  }
                                  // Release the chooser after use
                                  reclistChooser.reset();
                                });
  } else if (button == loadBGMButton.get()) {
    DBG("Load BGM button clicked");
    // Open file chooser for BGM files
    bgmChooser = std::make_unique<juce::FileChooser>("Select BGM File",
                                                     juce::File(), "*.wav");
    bgmChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                juce::FileBrowserComponent::canSelectFiles,
                            [this](const juce::FileChooser &f) {
                              auto result = f.getResult();
                              DBG("BGM file chosen: " +
                                  result.getFullPathName());
                              if (result.exists()) {
                                DBG("BGM file exists, loading...");
                                audioProcessor.loadGuideBGM(result);
                              } else {
                                DBG("BGM file does not exist");
                              }
                              // Release the chooser after use
                              bgmChooser.reset();
                            });
  }
}

void KiraNastroEditor::sliderValueChanged(juce::Slider *slider) {
#ifdef JUCE_STANDALONE_APPLICATION
  if (slider == progressSlider.get()) {
    // Set BGM playback position based on slider value
    const double newPos = slider->getValue();
    audioProcessor.seekBGM(newPos);
  }
#endif
}
