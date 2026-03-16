// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "LookAndFeel.h"
#include "Md3ExpressiveComboBox.h"

// First-run project setup screen.
// Displayed on first plugin launch and accessible from the menu.
class ProjectSetupScreen : public juce::Component
{
public:
    struct SetupResult
    {
        enum class Source
        {
            Builtin,
            Custom
        };
        Source reclistSource = Source::Builtin;
        int builtinReclistId = 0;
        juce::File customReclistFile;

        Source bgmSource = Source::Builtin;
        int builtinBGMTempo = 120;
        juce::String builtinBGMKey = "G";
        juce::File customBGMFile;
    };

    ProjectSetupScreen();
    ~ProjectSetupScreen() override;

    // Pre-populate selections from the current processor state
    void setInitialReclistSelection(bool isBuiltin, int builtinId, const juce::File &customFile = {});
    void setInitialBGMSelection(bool isBuiltin, int tempo, const juce::String &key,
                                const juce::File &customFile = {});

    std::function<void(const SetupResult &)> onComplete;

    //==============================================================================
    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    // Reclist panel
    std::unique_ptr<Md3ExpressiveComboBox> m_reclistCombo;
    std::unique_ptr<juce::TextButton> m_reclistCustomButton;
    std::unique_ptr<juce::Component> m_reclistCustomCard; // shown when custom file selected
    juce::Label m_reclistCustomLabel;
    std::unique_ptr<juce::DrawableButton> m_reclistClearButton;

    // BGM panel
    std::unique_ptr<Md3ExpressiveComboBox> m_tempoCombo;
    std::unique_ptr<Md3ExpressiveComboBox> m_keyCombo;
    std::unique_ptr<juce::TextButton> m_bgmCustomButton;
    std::unique_ptr<juce::Component> m_bgmCustomCard;
    juce::Label m_bgmCustomLabel;
    std::unique_ptr<juce::DrawableButton> m_bgmClearButton;

    // OK button (MD3e Icon Button M — 56×56 filled circle)
    std::unique_ptr<juce::DrawableButton> m_fabButton;
    std::unique_ptr<juce::Drawable> m_checkIcon;
    std::unique_ptr<juce::Drawable> m_logoDrawable;

    // File choosers
    std::unique_ptr<juce::FileChooser> m_reclistChooser;
    std::unique_ptr<juce::FileChooser> m_bgmChooser;

    // State
    bool m_usingCustomReclist = false;
    juce::File m_customReclistFile;

    bool m_usingCustomBGM = false;
    juce::File m_customBGMFile;

    void buildUI();
    void updateReclistPanel();
    void updateBGMPanel();
    void onConfirm();

    KiraNastroLookAndFeel *getLAF() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProjectSetupScreen)
};
