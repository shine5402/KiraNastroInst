// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "LookAndFeel.h"

// Material Design 3 "Expressive" ComboBox.
// Shows an always-visible header with overline label + selected value + chevron.
// On click, opens a PopupMenu dropdown anchored below the header.
class Md3ExpressiveComboBox : public juce::Component
{
public:
    struct ComboItem
    {
        int id;
        juce::String primaryText;
        juce::String supportingText; // empty = one-line (48px), non-empty = two-line (64px)
    };

    explicit Md3ExpressiveComboBox(const juce::String &overlineLabel);
    ~Md3ExpressiveComboBox() override;

    void addItem(int id, const juce::String &primary, const juce::String &supporting = {});
    void clearItems();
    void setSelectedId(int id, juce::NotificationType notify = juce::sendNotification);
    int getSelectedId() const { return m_selectedId; }
    juce::String getSelectedText() const;

    std::function<void(int)> onChange;

    //==============================================================================
    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent &e) override;

private:
    juce::String m_overlineLabel;
    std::vector<ComboItem> m_items;
    int m_selectedId = -1;
    bool m_dropdownOpen = false;
    juce::uint32 m_lastCloseTime = 0;

    std::unique_ptr<juce::Drawable> m_chevronIcon;

    void showDropdown();

    KiraNastroLookAndFeel *getLAF() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Md3ExpressiveComboBox)
};
