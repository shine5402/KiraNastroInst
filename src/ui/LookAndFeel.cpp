#include "LookAndFeel.h"

#include "../utils/Fonts.h"

// MD3 Light Scheme
const juce::Colour KiraNastroLookAndFeel::md3Background{0xFFFBF8FF};
const juce::Colour KiraNastroLookAndFeel::md3CardFilled{0xFFF4F2FA};
const juce::Colour KiraNastroLookAndFeel::md3Primary{0xFF4E5B92};
const juce::Colour KiraNastroLookAndFeel::md3OnSurfaceVariant{0xFF45464F};
const juce::Colour KiraNastroLookAndFeel::md3SecondaryContainer{0xFFDEE1F9};
const juce::Colour KiraNastroLookAndFeel::md3OnSecondaryContainer{0xFF424659};
const juce::Colour KiraNastroLookAndFeel::md3SurfaceVariant{0xFFE2E1EC};
const juce::Colour KiraNastroLookAndFeel::md3NavBar{0xFF3755C3};
const juce::Colour KiraNastroLookAndFeel::md3OnNavBar{0xFFFFFFFF};

// MD3 Dark Scheme
const juce::Colour KiraNastroLookAndFeel::md3BackgroundDark{0xFF121318};
const juce::Colour KiraNastroLookAndFeel::md3CardFilledDark{0xFF1A1B21};
const juce::Colour KiraNastroLookAndFeel::md3PrimaryDark{0xFFB8C4FF};
const juce::Colour KiraNastroLookAndFeel::md3OnSurfaceVariantDark{0xFFC6C5D0};
const juce::Colour KiraNastroLookAndFeel::md3SecondaryContainerDark{0xFF424659};
const juce::Colour KiraNastroLookAndFeel::md3OnSecondaryContainerDark{0xFFDEE1F9};
const juce::Colour KiraNastroLookAndFeel::md3SurfaceVariantDark{0xFF45464F};
const juce::Colour KiraNastroLookAndFeel::md3NavBarDark{0xFF1E2A5E};
const juce::Colour KiraNastroLookAndFeel::md3OnNavBarDark{0xFFFFFFFF};

KiraNastroLookAndFeel::KiraNastroLookAndFeel()
{
    // Apply MD3 palette to standard JUCE colour IDs
    setColour(juce::ResizableWindow::backgroundColourId, md3Background);
    setColour(juce::TextButton::buttonColourId, md3Primary);
    setColour(juce::TextButton::textColourOffId, md3OnNavBar);
    setColour(juce::Label::textColourId, md3Primary);
    setColour(juce::ScrollBar::thumbColourId, md3Primary);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colours::white);
    setColour(juce::PopupMenu::textColourId, md3Primary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, md3Primary.withAlpha(0.1f));
    setColour(juce::PopupMenu::highlightedTextColourId, md3Primary);
}

void KiraNastroLookAndFeel::setDarkMode(bool dark)
{
    m_isDark = dark;

    // Re-apply JUCE colour IDs using the new scheme
    setColour(juce::ResizableWindow::backgroundColourId, background());
    setColour(juce::TextButton::buttonColourId, primary());
    setColour(juce::TextButton::textColourOffId, onNavBar());
    setColour(juce::Label::textColourId, primary());
    setColour(juce::ScrollBar::thumbColourId, primary());
    setColour(juce::PopupMenu::backgroundColourId, m_isDark ? cardFilled() : juce::Colours::white);
    setColour(juce::PopupMenu::textColourId, primary());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, primary().withAlpha(0.1f));
    setColour(juce::PopupMenu::highlightedTextColourId, primary());
}

// Accessor methods that respect isDark state
juce::Colour KiraNastroLookAndFeel::background() const
{
    return m_isDark ? md3BackgroundDark : md3Background;
}
juce::Colour KiraNastroLookAndFeel::cardFilled() const
{
    return m_isDark ? md3CardFilledDark : md3CardFilled;
}
juce::Colour KiraNastroLookAndFeel::primary() const
{
    return m_isDark ? md3PrimaryDark : md3Primary;
}
juce::Colour KiraNastroLookAndFeel::onSurfaceVariant() const
{
    return m_isDark ? md3OnSurfaceVariantDark : md3OnSurfaceVariant;
}
juce::Colour KiraNastroLookAndFeel::secondaryContainer() const
{
    return m_isDark ? md3SecondaryContainerDark : md3SecondaryContainer;
}
juce::Colour KiraNastroLookAndFeel::onSecondaryContainer() const
{
    return m_isDark ? md3OnSecondaryContainerDark : md3OnSecondaryContainer;
}
juce::Colour KiraNastroLookAndFeel::surfaceVariant() const
{
    return m_isDark ? md3SurfaceVariantDark : md3SurfaceVariant;
}
juce::Colour KiraNastroLookAndFeel::navBar() const
{
    return m_isDark ? md3NavBarDark : md3NavBar;
}
juce::Colour KiraNastroLookAndFeel::onNavBar() const
{
    return m_isDark ? md3OnNavBarDark : md3OnNavBar;
}

void KiraNastroLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                                 const juce::Colour &backgroundColour,
                                                 bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    const float cornerRadius = static_cast<float>(button.getHeight()) / 2.0f;

    juce::Colour fillColour = backgroundColour;
    if (shouldDrawButtonAsDown)
        fillColour = fillColour.overlaidWith(juce::Colours::white.withAlpha(0.2f));
    else if (shouldDrawButtonAsHighlighted)
        fillColour = fillColour.overlaidWith(juce::Colours::white.withAlpha(0.1f));

    g.setColour(fillColour);
    g.fillRoundedRectangle(bounds, cornerRadius);
}

void KiraNastroLookAndFeel::drawLabel(juce::Graphics &g, juce::Label &label)
{
    LookAndFeel_V4::drawLabel(g, label);
}

void KiraNastroLookAndFeel::drawPopupMenuBackground(juce::Graphics &g, int width, int height)
{
    const juce::Rectangle<float> bounds(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    g.setColour(m_isDark ? cardFilled() : juce::Colours::white);
    g.fillRoundedRectangle(bounds, 12.0f);

    g.setColour(primary().withAlpha(0.15f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 12.0f, 1.0f);
}

void KiraNastroLookAndFeel::drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area, bool isSeparator,
                                              bool isActive, bool isHighlighted, bool /*isTicked*/, bool /*hasSubMenu*/,
                                              const juce::String &text, const juce::String & /*shortcutKeyText*/,
                                              const juce::Drawable * /*icon*/, const juce::Colour * /*textColour*/)
{
    if (isSeparator) {
        g.setColour(primary().withAlpha(0.15f));
        g.fillRect(area.getX() + 8, area.getCentreY(), area.getWidth() - 16, 1);
        return;
    }

    if (isHighlighted && isActive) {
        g.setColour(primary().withAlpha(0.10f));
        g.fillRect(area);
    }

    const juce::Colour textCol = isActive ? primary() : juce::Colours::grey;
    g.setColour(textCol);
    g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(14.0f)));
    g.drawFittedText(text, area.withLeft(area.getX() + 16).withRight(area.getRight() - 8),
                     juce::Justification::centredLeft, 1);
}

int KiraNastroLookAndFeel::getPopupMenuBorderSize()
{
    return 4;
}
