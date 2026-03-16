// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Md3ExpressiveComboBox.h"

#include "../utils/Fonts.h"
#include "../utils/Icons.h"

//==============================================================================
Md3ExpressiveComboBox::Md3ExpressiveComboBox(const juce::String &overlineLabel)
    : m_overlineLabel(overlineLabel)
{
    m_chevronIcon = Icons::load(Icons::chevronDownSvg);
    setInterceptsMouseClicks(true, false);
}

Md3ExpressiveComboBox::~Md3ExpressiveComboBox() = default;

void Md3ExpressiveComboBox::addItem(int id, const juce::String &primary, const juce::String &supporting)
{
    m_items.push_back({id, primary, supporting});
}

void Md3ExpressiveComboBox::clearItems()
{
    m_items.clear();
    m_selectedId = -1;
}

void Md3ExpressiveComboBox::setSelectedId(int id, juce::NotificationType notify)
{
    if (m_selectedId == id)
        return;
    m_selectedId = id;
    repaint();
    if (notify == juce::sendNotification && onChange)
        onChange(id);
}

juce::String Md3ExpressiveComboBox::getSelectedText() const
{
    for (const auto &item : m_items) {
        if (item.id == m_selectedId)
            return item.primaryText;
    }
    return {};
}

KiraNastroLookAndFeel *Md3ExpressiveComboBox::getLAF() const
{
    return dynamic_cast<KiraNastroLookAndFeel *>(&getLookAndFeel());
}

void Md3ExpressiveComboBox::paint(juce::Graphics &g)
{
    auto *laf = getLAF();
    const auto bg = laf ? laf->surfaceContainerLowest() : juce::Colours::white;
    const auto onSurface = laf ? laf->onSurface() : juce::Colours::black;
    const auto onSurfaceVariant = laf ? laf->onSurfaceVariant() : juce::Colours::darkgrey;
    const auto surfCont = laf ? laf->surfaceContainer() : juce::Colour(0xFFEFEDF4);

    auto bounds = getLocalBounds().toFloat();

    // Card background — no stroke
    g.setColour(bg);
    g.fillRoundedRectangle(bounds, 16.0f);

    // Overline label
    g.setColour(onSurfaceVariant);
    g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(11.0f)));
    g.drawFittedText(m_overlineLabel.toUpperCase(),
                     juce::Rectangle<int>(16, 8, getWidth() - 56, 14),
                     juce::Justification::centredLeft, 1);

    // Selected value
    g.setColour(onSurface);
    g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(16.0f)));
    g.drawFittedText(getSelectedText(),
                     juce::Rectangle<int>(16, 24, getWidth() - 56, 20),
                     juce::Justification::centredLeft, 1);

    // Chevron icon button — pill-shaped container (32×40, rx=16) per MD3 spec
    if (m_chevronIcon) {
        const float btnW = 32.0f;
        const float btnH = 40.0f;
        const float btnX = static_cast<float>(getWidth()) - btnW - 8.0f;
        const float btnY = (static_cast<float>(getHeight()) - btnH) / 2.0f;

        // Pill background — different color when expanded
        g.setColour(m_dropdownOpen ? surfCont : bg);
        g.fillRoundedRectangle(btnX, btnY, btnW, btnH, 16.0f);

        // Chevron icon (12px, centered in pill)
        const float iconSize = 12.0f;
        const float iconX = btnX + (btnW - iconSize) / 2.0f;
        const float iconY = btnY + (btnH - iconSize) / 2.0f;

        m_chevronIcon->replaceColour(juce::Colour(0xFF1F1F1F), onSurfaceVariant);

        if (m_dropdownOpen) {
            juce::Graphics::ScopedSaveState sss(g);
            g.addTransform(juce::AffineTransform::rotation(
                juce::MathConstants<float>::pi,
                iconX + iconSize / 2.0f,
                iconY + iconSize / 2.0f));
            m_chevronIcon->drawWithin(g,
                                      juce::Rectangle<float>(iconX, iconY, iconSize, iconSize),
                                      juce::RectanglePlacement::centred, 1.0f);
        } else {
            m_chevronIcon->drawWithin(g,
                                      juce::Rectangle<float>(iconX, iconY, iconSize, iconSize),
                                      juce::RectanglePlacement::centred, 1.0f);
        }
    }
}

void Md3ExpressiveComboBox::resized()
{
}

void Md3ExpressiveComboBox::mouseDown(const juce::MouseEvent &)
{
    // Prevent re-opening immediately after the popup was dismissed by clicking on us
    if (juce::Time::getMillisecondCounter() - m_lastCloseTime < 300)
        return;

    if (!m_dropdownOpen)
        showDropdown();
}

void Md3ExpressiveComboBox::showDropdown()
{
    if (m_dropdownOpen || m_items.empty())
        return;

    m_dropdownOpen = true;
    repaint();

    if (auto *laf = getLAF())
        laf->setComboPopupActive(true);

    juce::PopupMenu menu;
    for (const auto &item : m_items) {
        // Encode supporting text after \n for the LookAndFeel to render as two lines
        juce::String menuText = item.primaryText;
        if (item.supportingText.isNotEmpty())
            menuText += "\n" + item.supportingText;

        // Offset ID by 1 — PopupMenu reserves 0 for "no selection"
        const int menuId = item.id + 1;
        const bool selected = (item.id == m_selectedId);
        menu.addItem(menuId, menuText, true, selected);
    }

    // Position the menu below the combo box
    auto options = juce::PopupMenu::Options()
                       .withTargetComponent(this)
                       .withMinimumWidth(getWidth())
                       .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::downwards);

    menu.showMenuAsync(options, [safeThis = juce::Component::SafePointer<Md3ExpressiveComboBox>(this)](int result) {
        if (auto *self = safeThis.getComponent()) {
            if (auto *laf = self->getLAF())
                laf->setComboPopupActive(false);
            self->m_dropdownOpen = false;
            self->m_lastCloseTime = juce::Time::getMillisecondCounter();
            self->repaint();
            if (result > 0)
                self->setSelectedId(result - 1, juce::sendNotification);
        }
    });
}
