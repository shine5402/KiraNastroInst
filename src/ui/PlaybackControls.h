#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class KiraNastroProcessor;

class PlaybackControls : public juce::Component, public juce::Button::Listener
{
public:
    PlaybackControls(KiraNastroProcessor& processor);
    ~PlaybackControls() override = default;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    
private:
    KiraNastroProcessor& processor;
    
    std::unique_ptr<juce::TextButton> playButton;
    std::unique_ptr<juce::TextButton> stopButton;
    std::unique_ptr<juce::TextButton> nextButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackControls)
};