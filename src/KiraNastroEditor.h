// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "KiraNastroProcessor.h"
#include "ui/LookAndFeel.h"
#include "ui/PlaybackControls.h"
#include "ui/ProjectSetupScreen.h"
#include "ui/TimingIndicator.h"

// Constrainer that only allows horizontal (width) resizing.
// Height is locked during user drags; programmatic setSize() still works.
class WidthOnlyConstrainer : public juce::ComponentBoundsConstrainer
{
public:
    void setFixedHeight(int h) { m_fixedHeight = h; }

    void checkBounds(juce::Rectangle<int> &bounds,
                     const juce::Rectangle<int> &previousBounds,
                     const juce::Rectangle<int> &limits,
                     bool isStretchingTop, bool isStretchingLeft,
                     bool isStretchingBottom, bool isStretchingRight) override
    {
        ComponentBoundsConstrainer::checkBounds(bounds, previousBounds, limits,
                                                isStretchingTop, isStretchingLeft,
                                                isStretchingBottom, isStretchingRight);
        // Lock height to the managed value
        bounds.setHeight(m_fixedHeight);
        bounds.setY(previousBounds.getY());
    }

private:
    int m_fixedHeight = 232;
};

class KiraNastroEditor : public juce::AudioProcessorEditor, public juce::Timer, public juce::Slider::Listener
{
public:
    explicit KiraNastroEditor(KiraNastroProcessor &);
    ~KiraNastroEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    //==============================================================================
    void timerCallback() override;
    void sliderValueChanged(juce::Slider *) override;
    void mouseDown(const juce::MouseEvent &event) override;

private:
    KiraNastroProcessor &m_audioProcessor;
    KiraNastroLookAndFeel m_lookAndFeel;
    WidthOnlyConstrainer m_constrainer;

    // Cached state for change detection
    int m_lastEntryIndex = -1;
    juce::String m_currentEntryName;
    juce::String m_currentEntryComment;
    juce::String m_nextEntryName;
    juce::String m_nextEntryComment;
    float m_cachedUtteranceStartFraction = -1.0f; // -1 forces initial push
    float m_cachedUtteranceEndFraction   = -1.0f;

    // File chooser for export
    std::unique_ptr<juce::FileChooser> m_descChooser;

    // Hamburger menu button (always present)
    std::unique_ptr<juce::DrawableButton> m_menuButton;

    // Brand logo loaded from SVG
    std::unique_ptr<juce::Drawable> m_logoDrawable;

    // Chip icons (Material Symbols, tinted onSecondaryContainer)
    std::unique_ptr<juce::Drawable> m_nextIcon;
    std::unique_ptr<juce::Drawable> m_progressIcon;

    // Timing pie chart (always present)
    std::unique_ptr<TimingIndicator> m_timingIndicator;

    // Setup screen (shown on first launch or from menu)
    std::unique_ptr<ProjectSetupScreen> m_setupScreen;
    bool m_showingSetup = false;

    // Standalone-only debug controls (null in plugin mode; created at runtime
    // based on wrapperType — compile-time guards don't work in JUCE shared code)
    std::unique_ptr<PlaybackControls> m_playbackControls;
    std::unique_ptr<juce::Slider> m_progressSlider;

    void showMenu();
    void reloadChipIcons();
    void showSetupScreen();
    void hideSetupScreen();
    void applySetupResult(const ProjectSetupScreen::SetupResult &result);
    void enforceMinWidthForEntry();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroEditor)
};
