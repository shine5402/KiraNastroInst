// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PlaybackControls.h"

#include "../KiraNastroProcessor.h"
#include "../utils/Icons.h"

namespace
{
constexpr int kIconSize = 12;
constexpr int kHitSize  = 28; // hit area / state layer circle diameter
constexpr int kEdgeIndent = (kHitSize - kIconSize) / 2; // 8px padding around icon
constexpr int kSpacing  = 4;

const juce::Colour kIconColour(0xFF45464F);

std::unique_ptr<juce::DrawableButton> makeIconButton(
    const char *name, const char *svgText)
{
    auto normal  = Icons::load(svgText, kIconColour.withAlpha(0.55f));
    auto hover   = Icons::load(svgText, kIconColour.withAlpha(0.80f));
    auto pressed = Icons::load(svgText, kIconColour);
    auto btn = std::make_unique<juce::DrawableButton>(name, juce::DrawableButton::ImageFitted);
    btn->setImages(normal.get(), hover.get(), pressed.get());
    btn->setEdgeIndent(kEdgeIndent);
    return btn;
}
} // namespace

PlaybackControls::PlaybackControls(KiraNastroProcessor &processor) : m_processor(processor)
{
    // Cache play/stop icon sets (3 alpha levels each)
    m_playNormal  = Icons::load(Icons::playArrowSvg, kIconColour.withAlpha(0.55f));
    m_playHover   = Icons::load(Icons::playArrowSvg, kIconColour.withAlpha(0.80f));
    m_playPressed = Icons::load(Icons::playArrowSvg, kIconColour);
    m_stopNormal  = Icons::load(Icons::stopSvg, kIconColour.withAlpha(0.55f));
    m_stopHover   = Icons::load(Icons::stopSvg, kIconColour.withAlpha(0.80f));
    m_stopPressed = Icons::load(Icons::stopSvg, kIconColour);

    // Play/Stop toggle
    m_playStopButton = std::make_unique<juce::DrawableButton>("playStop", juce::DrawableButton::ImageFitted);
    m_playStopButton->setEdgeIndent(kEdgeIndent);
    m_playStopButton->setImages(m_playNormal.get(), m_playHover.get(), m_playPressed.get());
    m_playStopButton->onClick = [this] {
        if (m_processor.isBGMPlaying())
            m_processor.stopBGM();
        else
            m_processor.startBGM();
    };
    m_playStopButton->onStateChange = [this] { repaint(); };
    addAndMakeVisible(m_playStopButton.get());

    // Previous entry
    m_prevButton = makeIconButton("prev", Icons::skipPreviousSvg);
    m_prevButton->onClick = [this] {
        const double cycle = m_processor.getBGMLengthSeconds();
        if (cycle <= 0.0)
            return;
        const double currentPos = m_processor.m_projectPlayPositionSeconds.load();
        const double prevStart = std::max(0.0, (std::floor(currentPos / cycle) - 1.0) * cycle);
        m_processor.seekBGM(prevStart);
    };
    m_prevButton->onStateChange = [this] { repaint(); };
    addAndMakeVisible(m_prevButton.get());

    // Next entry
    m_nextButton = makeIconButton("next", Icons::skipNextSvg);
    m_nextButton->onClick = [this] {
        const double cycle = m_processor.getBGMLengthSeconds();
        if (cycle <= 0.0)
            return;
        const double currentPos = m_processor.m_projectPlayPositionSeconds.load();
        m_processor.seekBGM(std::floor(currentPos / cycle + 1.1) * cycle);
    };
    m_nextButton->onStateChange = [this] { repaint(); };
    addAndMakeVisible(m_nextButton.get());

    startTimerHz(10);
}

void PlaybackControls::paint(juce::Graphics &g)
{
    // Draw circular state layers behind each button (MD3 style)
    drawStateLayer(g, m_prevButton.get());
    drawStateLayer(g, m_playStopButton.get());
    drawStateLayer(g, m_nextButton.get());
}

void PlaybackControls::resized()
{
    const int totalW = kHitSize * 3 + kSpacing * 2;
    int x = (getWidth() - totalW) / 2;
    int y = (getHeight() - kHitSize) / 2;

    m_prevButton->setBounds(x, y, kHitSize, kHitSize);
    x += kHitSize + kSpacing;
    m_playStopButton->setBounds(x, y, kHitSize, kHitSize);
    x += kHitSize + kSpacing;
    m_nextButton->setBounds(x, y, kHitSize, kHitSize);
}

void PlaybackControls::timerCallback()
{
    updatePlayStopIcon();
}

void PlaybackControls::updatePlayStopIcon()
{
    const bool playing = m_processor.isBGMPlaying();
    if (playing != m_wasPlaying) {
        m_wasPlaying = playing;
        if (playing)
            m_playStopButton->setImages(m_stopNormal.get(), m_stopHover.get(), m_stopPressed.get());
        else
            m_playStopButton->setImages(m_playNormal.get(), m_playHover.get(), m_playPressed.get());
    }
}

void PlaybackControls::drawStateLayer(juce::Graphics &g, juce::DrawableButton *btn)
{
    if (!btn || (!btn->isOver() && !btn->isDown()))
        return;
    auto circle = btn->getBounds().toFloat().withSizeKeepingCentre(
        static_cast<float>(kHitSize), static_cast<float>(kHitSize));
    g.setColour(kIconColour.withAlpha(btn->isDown() ? 0.10f : 0.08f));
    g.fillEllipse(circle);
}
