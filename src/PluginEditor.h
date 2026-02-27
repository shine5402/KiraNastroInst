#pragma once

#include "PluginProcessor.h"
#include "ui/LookAndFeel.h"
#include "ui/TimingIndicator.h"
#include <juce_audio_processors/juce_audio_processors.h>

class KiraNastroEditor : public juce::AudioProcessorEditor,
                         public juce::Timer,
                         public juce::Button::Listener,
                         public juce::Slider::Listener {
public:
  explicit KiraNastroEditor(KiraNastroProcessor &);
  ~KiraNastroEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

  //==============================================================================
  void timerCallback() override;
  void buttonClicked(juce::Button *) override;
  void sliderValueChanged(juce::Slider *) override;

private:
  KiraNastroProcessor &audioProcessor;
  KiraNastroLookAndFeel lookAndFeel;

  // Cached state for change detection
  int lastEntryIndex = -1;

  // Configuration UI
  std::unique_ptr<juce::TextButton> loadReclistButton;
  std::unique_ptr<juce::TextButton> loadBGMButton;

  // File choosers
  std::unique_ptr<juce::FileChooser> reclistChooser;
  std::unique_ptr<juce::FileChooser> bgmChooser;

// Standalone-specific controls
#ifdef JUCE_STANDALONE_APPLICATION
  std::unique_ptr<class PlaybackControls> playbackControls;
  std::unique_ptr<juce::Slider> progressSlider;
  std::unique_ptr<TimingIndicator> timingIndicator;
#endif

  juce::String currentEntryName;
  juce::String currentEntryComment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroEditor)
};
