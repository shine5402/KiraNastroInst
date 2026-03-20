// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LookAndFeel.h"

#include "../utils/Fonts.h"

#if JUCE_WINDOWS
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

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
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainerHighest{0xFFE9E7EF}; // Same as High for light
const juce::Colour KiraNastroLookAndFeel::md3OnSurface{0xFF1A1B21};
const juce::Colour KiraNastroLookAndFeel::md3OutlineVariant{0xFFC6C5D0};

// MD3 Surface / Outline tokens (dark)
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainerDark{0xFF1F1F25};
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainerHighDark{0xFF292A2F};
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainerHighestDark{0xFF292A2F}; // Same as High for dark
const juce::Colour KiraNastroLookAndFeel::md3OnSurfaceDark{0xFFE3E1E9};
const juce::Colour KiraNastroLookAndFeel::md3OutlineVariantDark{0xFF45464F};

// MD3 Tertiary Container tokens
const juce::Colour KiraNastroLookAndFeel::md3TertiaryContainer{0xFFCEE5FF};
const juce::Colour KiraNastroLookAndFeel::md3TertiaryContainerDark{0xFF254A69};

// MD3 Scrim token (same for both themes)
const juce::Colour KiraNastroLookAndFeel::md3Scrim{0x52000000};

// MD3 Primary Container tokens
const juce::Colour KiraNastroLookAndFeel::md3PrimaryContainer{0xFFDDE1FF};
const juce::Colour KiraNastroLookAndFeel::md3PrimaryContainerDark{0xFF364379};
const juce::Colour KiraNastroLookAndFeel::md3OnPrimaryContainer{0xFF364379};
const juce::Colour KiraNastroLookAndFeel::md3OnPrimaryContainerDark{0xFFDDE1FF};

// MD3 Surface Container Lowest tokens
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainerLowest{0xFFFFFFFF};
const juce::Colour KiraNastroLookAndFeel::md3SurfaceContainerLowestDark{0xFF0E0F14};

// MD3 OnPrimary tokens
const juce::Colour KiraNastroLookAndFeel::md3OnPrimary{0xFFFFFFFF};
const juce::Colour KiraNastroLookAndFeel::md3OnPrimaryDark{0xFF1F2D61};

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
    setColour(juce::PopupMenu::highlightedBackgroundColourId, md3OnSurface.withAlpha(0.08f));
    setColour(juce::PopupMenu::highlightedTextColourId, md3OnSurface);
    setColour(juce::AlertWindow::backgroundColourId, md3SurfaceContainerHigh);
    setColour(juce::AlertWindow::textColourId, md3OnSurfaceVariant);
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
    setColour(juce::PopupMenu::highlightedBackgroundColourId, onSurface().withAlpha(0.08f));
    setColour(juce::PopupMenu::highlightedTextColourId, onSurface());
    setColour(juce::AlertWindow::backgroundColourId, surfaceContainerHigh());
    setColour(juce::AlertWindow::textColourId, onSurfaceVariant());
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
juce::Colour KiraNastroLookAndFeel::surfaceContainerHighest() const
{
    return m_isDark ? md3SurfaceContainerHighestDark : md3SurfaceContainerHighest;
}
juce::Colour KiraNastroLookAndFeel::onSurface() const
{
    return m_isDark ? md3OnSurfaceDark : md3OnSurface;
}
juce::Colour KiraNastroLookAndFeel::outlineVariant() const
{
    return m_isDark ? md3OutlineVariantDark : md3OutlineVariant;
}

juce::Colour KiraNastroLookAndFeel::tertiaryContainer() const
{
    return m_isDark ? md3TertiaryContainerDark : md3TertiaryContainer;
}

juce::Colour KiraNastroLookAndFeel::scrim() const
{
    return md3Scrim;
}
juce::Colour KiraNastroLookAndFeel::surfaceContainerLowest() const
{
    return m_isDark ? md3SurfaceContainerLowestDark : md3SurfaceContainerLowest;
}
juce::Colour KiraNastroLookAndFeel::primaryContainer() const
{
    return m_isDark ? md3PrimaryContainerDark : md3PrimaryContainer;
}
juce::Colour KiraNastroLookAndFeel::onPrimaryContainer() const
{
    return m_isDark ? md3OnPrimaryContainerDark : md3OnPrimaryContainer;
}
juce::Colour KiraNastroLookAndFeel::onPrimary() const
{
    return m_isDark ? md3OnPrimaryDark : md3OnPrimary;
}

void KiraNastroLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                                 const juce::Colour &backgroundColour,
                                                 bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    const float cornerRadius = static_cast<float>(button.getHeight()) / 2.0f;

    // Check if this is a dialog button (AlertWindow or MD3Dialog)
    auto *parent = button.getParentComponent();
    bool isDialogButton = (dynamic_cast<juce::AlertWindow *>(parent) != nullptr) ||
                          (parent != nullptr && parent->getComponentID() == "md3Dialog");

    // MD3 text button inside dialog — transparent bg, state layer only on interaction
    if (isDialogButton)
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
    // Check if this is a dialog button (AlertWindow or MD3Dialog)
    auto *parent = button.getParentComponent();
    bool isDialogButton = (dynamic_cast<juce::AlertWindow *>(parent) != nullptr) ||
                          (parent != nullptr && parent->getComponentID() == "md3Dialog");

    // MD3 text button inside dialog — labelLarge, primary color
    if (isDialogButton)
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
    g.setColour(m_comboPopupActive ? surfaceContainerLowest() : surfaceContainerHighest());
#if JUCE_WINDOWS
    // On Windows we let DWM handle rounded corners via DWMWCP_ROUND,
    // so fill the full rectangle — DWM clips to the native radius.
    g.fillRect(bounds);
#else
    g.fillRoundedRectangle(bounds, 16.0f);
#endif
    // No outline — MD3 menus have no border
}

void KiraNastroLookAndFeel::drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area, bool isSeparator,
                                              bool isActive, bool isHighlighted, bool isTicked, bool /*hasSubMenu*/,
                                              const juce::String &text, const juce::String & /*shortcutKeyText*/,
                                              const juce::Drawable * /*icon*/, const juce::Colour * /*textColour*/,
                                              bool isFirstItem, bool isLastItem)
{
    if (isSeparator) {
        g.setColour(outlineVariant());
        const int lineY = area.getCentreY();
        g.fillRect(area.getX() + 12, lineY, area.getWidth() - 24, 1);
        return;
    }

    // Helper: draw rounded rect with per-corner radii
    auto drawHighlightPath = [&](juce::Rectangle<float> hb, float tlr, float trr, float brr, float blr) {
        juce::Path path;
        const float x = hb.getX(), y = hb.getY(), w = hb.getWidth(), h = hb.getHeight();
        path.startNewSubPath(x + tlr, y);
        path.lineTo(x + w - trr, y);
        path.quadraticTo(x + w, y, x + w, y + trr);
        path.lineTo(x + w, y + h - brr);
        path.quadraticTo(x + w, y + h, x + w - brr, y + h);
        path.lineTo(x + blr, y + h);
        path.quadraticTo(x, y + h, x, y + h - blr);
        path.lineTo(x, y + tlr);
        path.quadraticTo(x, y, x + tlr, y);
        path.closeSubPath();
        g.fillPath(path);
    };

    auto highlightBounds = area.toFloat().reduced(2.0f, 0.0f);
#if JUCE_WINDOWS
    // Match Win11 native window corner radius (~8px)
    float topR = isFirstItem ? 8.0f : 4.0f;
    float botR = isLastItem ? 8.0f : 4.0f;
#else
    float topR = isFirstItem ? 16.0f : 4.0f;
    float botR = isLastItem ? 16.0f : 4.0f;
#endif

    // Selected (ticked) background — primaryContainer, 1px shrink top/bottom for gap
    // but NOT on the top of first item or bottom of last item
    if (isTicked) {
        auto selectedBounds = highlightBounds;
        if (!isFirstItem) selectedBounds.removeFromTop(1.0f);
        if (!isLastItem)  selectedBounds.removeFromBottom(1.0f);
        g.setColour(primaryContainer());
        drawHighlightPath(selectedBounds, topR, topR, botR, botR);
    }

    // Hover overlay — same shrink logic
    if (isHighlighted && isActive) {
        auto hoverBounds = highlightBounds;
        if (!isFirstItem) hoverBounds.removeFromTop(1.0f);
        if (!isLastItem)  hoverBounds.removeFromBottom(1.0f);
        g.setColour(onSurface().withAlpha(0.08f));
        drawHighlightPath(hoverBounds, topR, topR, botR, botR);
    }

    // Detect two-line item: text contains \n
    const bool twoLine = text.contains("\n");
    const auto primaryText = twoLine ? text.upToFirstOccurrenceOf("\n", false, false) : text;
    const auto supportText = twoLine ? text.fromFirstOccurrenceOf("\n", false, false) : juce::String();

    // Text colors
    const juce::Colour primaryCol = !isActive ? onSurface().withAlpha(0.38f)
                                   : isTicked ? onPrimaryContainer()
                                              : onSurface();

    auto textBounds = area.withLeft(area.getX() + 12).withRight(area.getRight() - 12);

    if (twoLine) {
        // Primary text — 16pt SemiBold
        g.setColour(primaryCol);
        g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(16.0f)));
        g.drawFittedText(primaryText,
                         textBounds.withHeight(22).withY(area.getY() + 10),
                         juce::Justification::centredLeft, 1);

        // Supporting text — 12pt, slightly muted
        const juce::Colour supportCol = !isActive ? onSurface().withAlpha(0.38f)
                                       : isTicked ? onPrimaryContainer().withAlpha(0.7f)
                                                  : onSurfaceVariant();
        g.setColour(supportCol);
        g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(12.0f)));
        g.drawFittedText(supportText,
                         textBounds.withHeight(18).withY(area.getY() + 34),
                         juce::Justification::centredLeft, 1);
    } else {
        // Single-line: MD3 labelLarge 14sp SemiBold
        g.setColour(primaryCol);
        g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(14.0f)));
        g.drawFittedText(primaryText, textBounds, juce::Justification::centredLeft, 1);
    }
}

void KiraNastroLookAndFeel::getIdealPopupMenuItemSize(const juce::String &text, bool isSeparator,
                                                       int /*standardMenuItemHeight*/, int &idealWidth,
                                                       int &idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 112; // M3 minimum width
        idealHeight = 9; // 1px line + 4dp top/bottom padding (more compact)
    }
    else
    {
        const bool twoLine = text.contains("\n");
        const auto primaryText = twoLine ? text.upToFirstOccurrenceOf("\n", false, false) : text;

        if (twoLine) {
            auto font = juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(16.0f));
            idealHeight = 64; // Two-line item height
            idealWidth = juce::GlyphArrangement::getStringWidthInt(font, primaryText) + 24;
        } else {
            auto font = juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(14.0f));
            idealHeight = 40; // Compact single-line item height
            idealWidth = juce::GlyphArrangement::getStringWidthInt(font, text) + 24;
        }

        idealWidth = juce::jmax(112, idealWidth);
    }
}

int KiraNastroLookAndFeel::getPopupMenuBorderSize()
{
    return 12; // MD3 horizontal padding (left/right)
}

juce::BorderSize<int> KiraNastroLookAndFeel::getPopupMenuBorderSizeAsBorder(const juce::PopupMenu::Options&)
{
    // MD3: 2px vertical padding, 12px horizontal padding
    return juce::BorderSize<int>(2, 12, 2, 12);
}

int KiraNastroLookAndFeel::getMenuWindowFlags()
{
#if JUCE_WINDOWS
    // Return 0 to prevent JUCE's software DropShadower (which draws a
    // rectangular shadow).  We set up native DWM shadow instead in
    // preparePopupMenuWindow().
    return 0;
#else
    return juce::ComponentPeer::windowHasDropShadow;
#endif
}

void KiraNastroLookAndFeel::preparePopupMenuWindow(juce::Component &newWindow)
{
#if JUCE_WINDOWS
    if (auto *peer = newWindow.getPeer()) {
        auto hwnd = static_cast<HWND>(peer->getNativeHandle());

        // Add WS_CAPTION so DWM treats this as a framed window.
        // JUCE's WM_NCCALCSIZE handler already removes the non-client area
        // for non-titled windows, so no caption bar will appear.
        auto style = GetWindowLongPtr(hwnd, GWL_STYLE);
        SetWindowLongPtr(hwnd, GWL_STYLE, style | WS_CAPTION);

        // Extend the DWM frame into the entire client area.  This lets DWM
        // draw the native shadow around the window.
        MARGINS margins = {-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        // Request Win11 native rounded corners — the shadow follows the
        // rounded shape automatically.
        constexpr DWORD DWMWA_WINDOW_CORNER_PREFERENCE_VALUE = 33;
        constexpr DWORD DWMWCP_ROUND_VALUE = 2;
        DWORD preference = DWMWCP_ROUND_VALUE;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE_VALUE,
                              &preference, sizeof(preference));

        // Force Windows to recalculate the frame.
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE
                         | SWP_NOZORDER | SWP_NOACTIVATE);
    }
#else
    juce::ignoreUnused(newWindow);
#endif
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
    return 48; // MD3 minimum touch target
}

juce::Font KiraNastroLookAndFeel::getAlertWindowTitleFont()
{
    // MD3 headlineSmall: 24sp, regular weight (NOT bold)
    // Line height: 32sp (handled by TextLayout)
    return juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(24.0f));
}

juce::Font KiraNastroLookAndFeel::getAlertWindowMessageFont()
{
    return juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(14.0f));
}
