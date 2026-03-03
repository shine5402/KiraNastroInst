// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PlaybackControls.h"

#include "../PluginProcessor.h"

PlaybackControls::PlaybackControls(KiraNastroProcessor &processor) : m_processor(processor)
{
    // Create buttons
    m_playButton = std::make_unique<juce::TextButton>("Play");
    m_stopButton = std::make_unique<juce::TextButton>("Stop");
    m_nextButton = std::make_unique<juce::TextButton>("Next");

    // Add and configure buttons
    addAndMakeVisible(m_playButton.get());
    addAndMakeVisible(m_stopButton.get());
    addAndMakeVisible(m_nextButton.get());

    m_playButton->addListener(this);
    m_stopButton->addListener(this);
    m_nextButton->addListener(this);

    m_playButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1A3FC7));
    m_playButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);

    m_stopButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFE74C3C));
    m_stopButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);

    m_nextButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF27AE60));
    m_nextButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);

    // Set the component's size last to avoid calling resized() before buttons are
    // ready
}

void PlaybackControls::paint(juce::Graphics &g)
{
    juce::ignoreUnused(g);
    // Optional: paint a background for the controls
}

void PlaybackControls::resized()
{
    if (m_playButton == nullptr || m_stopButton == nullptr || m_nextButton == nullptr)
        return;

    const int buttonWidth = 120;
    const int buttonHeight = 40;
    const int spacing = 16;
    const int totalWidth = (buttonWidth * 3) + (spacing * 2);

    int x = (getWidth() - totalWidth) / 2;
    int y = (getHeight() - buttonHeight) / 2;

    m_playButton->setBounds(x, y, buttonWidth, buttonHeight);
    x += buttonWidth + spacing;
    m_stopButton->setBounds(x, y, buttonWidth, buttonHeight);
    x += buttonWidth + spacing;
    m_nextButton->setBounds(x, y, buttonWidth, buttonHeight);
}

void PlaybackControls::buttonClicked(juce::Button *button)
{
    if (button == m_playButton.get()) {
        m_processor.startBGM();
    }
    else if (button == m_stopButton.get()) {
        m_processor.stopBGM();
    }
    else if (button == m_nextButton.get()) {
        const double cycle = m_processor.getBGMLengthSeconds();
        const double currentPos = m_processor.m_projectPlayPositionSeconds.load();

        // Seek to the start of the next cycle
        // (This will also trigger currentEntryIndex advancement internally in
        // seekBGM)
        m_processor.seekBGM(std::floor(currentPos / cycle + 1.1) * cycle);
    }
}