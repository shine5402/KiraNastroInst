// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_animation/juce_animation.h>

class MD3Dialog : public juce::Component
{
public:
    using Callback = std::function<void()>;

    MD3Dialog(const juce::String &title, const juce::String &message,
              const juce::String &buttonText, juce::Component *parent,
              Callback onClose = {});
    ~MD3Dialog() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent &) override {}

    static void show(const juce::String &title, const juce::String &message,
                     const juce::String &buttonText, juce::Component *parent,
                     Callback onClose = {});

private:
    void close();
    juce::Rectangle<int> computeDialogBounds() const;
    void startEnterAnimation();
    void startExitAnimation();
    void cleanupAndDelete();

    juce::Component *m_parent;
    juce::String m_title, m_message, m_buttonText;
    std::unique_ptr<juce::TextButton> m_button;
    Callback m_onClose;

    float m_animAlpha = 0.0f;
    float m_animScale = 0.85f;
    juce::Rectangle<int> m_targetBounds;
    
    std::optional<juce::Animator> m_spatialAnimator;
    std::optional<juce::Animator> m_effectsAnimator;
    std::unique_ptr<juce::VBlankAnimatorUpdater> m_updater;
    
    bool m_isExiting = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MD3Dialog)
};
