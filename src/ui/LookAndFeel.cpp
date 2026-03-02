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

// MD3 Surface / Outline tokens (light)
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainer{0xFFEFEDF4};
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainerHigh{0xFFE9E7EF};
const juce::Colour KiraNastroLookAndFeel::md3OnSurface{0xFF1A1B21};
const juce::Colour KiraNastroLookAndFeel::md3OutlineVariant{0xFFC6C5D0};

// MD3 Surface / Outline tokens (dark)
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainerDark{0xFF1F1F25};
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainerHighDark{0xFF292A2F};
const juce::Colour KiraNastroLookAndFeel::md3OnSurfaceDark{0xFFE3E1E9};
const juce::Colour KiraNastroLookAndFeel::md3OutlineVariantDark{0xFF45464F};

KiraNastroLookAndFeel::KiraNastroLookAndFeel()
{
    // Apply MD3 palette to standard JUCE colour IDs
    setColour(juce::ResizableWindow::backgroundColourId, md3Background);
    setColour(juce::TextButton::buttonColourId, md3Primary);
    setColour(juce::TextButton::textColourOffId, md3OnNavBar);
    setColour(juce::Label::textColourId, md3Primary);
    setColour(juce::ScrollBar::thumbColourId, md3Primary);
    // Use transparent so the MenuWindow is non-opaque — required for our rounded corners
    // to show correctly (the real fill colour is applied in drawPopupMenuBackground).
    setColour(juce::PopupMenu::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::PopupMenu::textColourId, md3OnSurface);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, md3OnSurface.withAlpha(0.10f));
    setColour(juce::PopupMenu::highlightedTextColourId, md3OnSurface);
    setColour(juce::AlertWindow::backgroundColourId, md3SurfaceContainerHigh);
    setColour(juce::AlertWindow::textColourId, md3OnSurface);
    setColour(juce::AlertWindow::outlineColourId, juce::Colours::transparentBlack);
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
    setColour(juce::PopupMenu::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::PopupMenu::textColourId, onSurface());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, onSurface().withAlpha(0.10f));
    setColour(juce::PopupMenu::highlightedTextColourId, onSurface());
    setColour(juce::AlertWindow::backgroundColourId, surfaceContainerHigh());
    setColour(juce::AlertWindow::textColourId, onSurface());
    setColour(juce::AlertWindow::outlineColourId, juce::Colours::transparentBlack);
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
juce::Colour KiraNastroLookAndFeel::surfaceContainer() const
{
    return m_isDark ? md3SurfaceContainerDark : md3SurfaceContainer;
}
juce::Colour KiraNastroLookAndFeel::surfaceContainerHigh() const
{
    return m_isDark ? md3SurfaceContainerHighDark : md3SurfaceContainerHigh;
}
juce::Colour KiraNastroLookAndFeel::onSurface() const
{
    return m_isDark ? md3OnSurfaceDark : md3OnSurface;
}
juce::Colour KiraNastroLookAndFeel::outlineVariant() const
{
    return m_isDark ? md3OutlineVariantDark : md3OutlineVariant;
}

void KiraNastroLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                                 const juce::Colour &backgroundColour,
                                                 bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    const float cornerRadius = static_cast<float>(button.getHeight()) / 2.0f;

    // MD3 text button inside AlertWindow — transparent bg, state layer only on interaction
    if (dynamic_cast<juce::AlertWindow *>(button.getParentComponent()) != nullptr)
    {
        if (shouldDrawButtonAsDown)
            g.setColour(primary().withAlpha(0.12f));
        else if (shouldDrawButtonAsHighlighted)
            g.setColour(primary().withAlpha(0.08f));
        else
            return;
        g.fillRoundedRectangle(bounds, cornerRadius);
        return;
    }

    // MD3 filled pill button (default)
    juce::Colour fillColour = backgroundColour;
    if (shouldDrawButtonAsDown)
        fillColour = fillColour.overlaidWith(juce::Colours::white.withAlpha(0.2f));
    else if (shouldDrawButtonAsHighlighted)
        fillColour = fillColour.overlaidWith(juce::Colours::white.withAlpha(0.1f));

    g.setColour(fillColour);
    g.fillRoundedRectangle(bounds, cornerRadius);
}

void KiraNastroLookAndFeel::drawButtonText(juce::Graphics &g, juce::TextButton &button,
                                           bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/)
{
    // MD3 text button inside AlertWindow — labelLarge, primary color
    if (dynamic_cast<juce::AlertWindow *>(button.getParentComponent()) != nullptr)
    {
        g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(14.0f)));
        g.setColour(primary());
        g.drawFittedText(button.getButtonText(), button.getLocalBounds(),
                         juce::Justification::centred, 1);
        return;
    }

    // Default: white text on filled pill
    g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(14.0f)));
    g.setColour(button.findColour(juce::TextButton::textColourOffId));
    g.drawFittedText(button.getButtonText(), button.getLocalBounds(),
                     juce::Justification::centred, 1);
}

void KiraNastroLookAndFeel::drawLabel(juce::Graphics &g, juce::Label &label)
{
    LookAndFeel_V4::drawLabel(g, label);
}

void KiraNastroLookAndFeel::drawPopupMenuBackground(juce::Graphics &g, int width, int height)
{
    const juce::Rectangle<float> bounds(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    g.setColour(surfaceContainer());
    g.fillRoundedRectangle(bounds, 4.0f);
    // No outline — MD3 menus have no border
}

void KiraNastroLookAndFeel::drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area, bool isSeparator,
                                              bool isActive, bool isHighlighted, bool /*isTicked*/, bool /*hasSubMenu*/,
                                              const juce::String &text, const juce::String & /*shortcutKeyText*/,
                                              const juce::Drawable * /*icon*/, const juce::Colour * /*textColour*/)
{
    if (isSeparator) {
        // MD3 "Vertical menu with divider" — full-width 1px line, no horizontal inset
        g.setColour(outlineVariant());
        g.fillRect(area.getX(), area.getCentreY(), area.getWidth(), 1);
        return;
    }

    if (isHighlighted && isActive) {
        g.setColour(onSurface().withAlpha(0.10f));
        g.fillRect(area);
    }

    const juce::Colour textCol = isActive ? onSurface() : onSurface().withAlpha(0.38f);
    g.setColour(textCol);
    // MD3 labelLarge: 14sp, weight 500 (Medium) → SemiBold is closest Sarasa weight
    g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(14.0f)));
    g.drawFittedText(text,
                     area.withLeft(area.getX() + 16).withRight(area.getRight() - 16),
                     juce::Justification::centredLeft, 1);
}

void KiraNastroLookAndFeel::getIdealPopupMenuItemSize(const juce::String &text, bool isSeparator,
                                                       int standardMenuItemHeight, int &idealWidth,
                                                       int &idealHeight)
{
    LookAndFeel_V4::getIdealPopupMenuItemSize(text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight);
    if (isSeparator)
        idealHeight = 17; // 1px line + 8dp top/bottom padding each side
    else
        idealHeight = juce::jmax(48, idealHeight); // MD3 minimum touch target
}

int KiraNastroLookAndFeel::getPopupMenuBorderSize()
{
    return 4;
}

void KiraNastroLookAndFeel::drawAlertBox(juce::Graphics &g, juce::AlertWindow &alert,
                                         const juce::Rectangle<int> &textArea,
                                         juce::TextLayout &textLayout)
{
    constexpr float cornerSize = 28.0f; // MD3 ExtraLarge shape

    // Background fill — no outline (MD3 dialogs rely on elevation, not border)
    g.setColour(surfaceContainerHigh());
    g.fillRoundedRectangle(alert.getLocalBounds().toFloat(), cornerSize);

    // Message text (title + body both in textLayout via attributed string).
    // JUCE passes textArea starting at edgeGap=10dp; add 14dp more to reach
    // the MD3-spec 24dp content margin. Non-NoIcon iconType gives topLeft
    // justification (left-aligned text) — we just don't draw the icon.
    g.setColour(alert.findColour(juce::AlertWindow::textColourId));
    textLayout.draw(g, textArea.withTrimmedLeft(14).toFloat());
}

int KiraNastroLookAndFeel::getAlertWindowButtonHeight()
{
    return 40;
}

juce::Font KiraNastroLookAndFeel::getAlertWindowTitleFont()
{
    // MD3 headlineSmall: 24sp, weight 475 → SemiBold is the closest Sarasa weight
    return juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(24.0f));
}

juce::Font KiraNastroLookAndFeel::getAlertWindowMessageFont()
{
    return juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(14.0f));
}
