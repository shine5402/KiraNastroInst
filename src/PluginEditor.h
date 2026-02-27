#pragma once

#include "PluginProcessor.h"
#include "ui/LookAndFeel.h"
#include "ui/TimingIndicator.h"
#include <juce_audio_processors/juce_audio_processors.h>

class KiraNastroEditor : public juce::AudioProcessorEditor,
                         public juce::Timer,
                         public juce::Slider::Listener {
public:
  explicit KiraNastroEditor(KiraNastroProcessor &);
  ~KiraNastroEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

  //==============================================================================
  void timerCallback() override;
  void sliderValueChanged(juce::Slider *) override;

private:
  KiraNastroProcessor &audioProcessor;
  KiraNastroLookAndFeel lookAndFeel;

  // Cached state for change detection
  int lastEntryIndex = -1;
  juce::String currentEntryName;
  juce::String currentEntryComment;
  juce::String nextEntryName;
  juce::String nextEntryComment;

  // File choosers
  std::unique_ptr<juce::FileChooser> reclistChooser;
  std::unique_ptr<juce::FileChooser> bgmChooser;

  // Hamburger menu button (always present)
  std::unique_ptr<juce::DrawableButton> menuButton;

  // Brand logo loaded from SVG
  std::unique_ptr<juce::Drawable> logoDrawable;

  // Chip icons (Material Symbols, tinted onSecondaryContainer)
  std::unique_ptr<juce::Drawable> nextIcon;
  std::unique_ptr<juce::Drawable> progressIcon;

  // Timing pie chart (always present)
  std::unique_ptr<TimingIndicator> timingIndicator;

// Standalone-specific controls
#ifdef JUCE_STANDALONE_APPLICATION
  std::unique_ptr<class PlaybackControls> playbackControls;
  std::unique_ptr<juce::Slider> progressSlider;
#endif

  void showMenu();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroEditor)
};
