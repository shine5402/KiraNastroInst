#include "PluginEditor.h"

KiraNastroEditor::KiraNastroEditor(KiraNastroProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(800, 300);
    startTimerHz(30);
}

KiraNastroEditor::~KiraNastroEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void KiraNastroEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFFF5F7FB));

    g.setColour(juce::Colour(0xFF1A3FC7));
    g.setFont(24.0f);
    g.drawFittedText("KiraNastro VSTi",
                     getLocalBounds(),
                     juce::Justification::centred,
                     1);
}

void KiraNastroEditor::resized()
{
    // TODO Phase 4: lay out MainComponent, TimingIndicator, bottom bar
}

//==============================================================================
void KiraNastroEditor::timerCallback()
{
    const int idx = audioProcessor.currentEntryIndex.load();
    if (idx != lastEntryIndex)
    {
        lastEntryIndex = idx;
        repaint();
    }
}
