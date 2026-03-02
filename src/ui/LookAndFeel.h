#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class KiraNastroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // MD3 Light Scheme tokens
    static const juce::Colour md3Background;           // #FBF8FF
    static const juce::Colour md3CardFilled;           // #F4F2FA (surfaceContainerLow)
    static const juce::Colour md3Primary;              // #4E5B92
    static const juce::Colour md3OnSurfaceVariant;     // #45464F
    static const juce::Colour md3SecondaryContainer;   // #DEE1F9
    static const juce::Colour md3OnSecondaryContainer; // #424659
    static const juce::Colour md3SurfaceVariant;       // #E2E1EC
    static const juce::Colour md3NavBar;               // #3755C3 (Primary 40)
    static const juce::Colour md3OnNavBar;             // #FFFFFF

    // MD3 Dark Scheme tokens (foundation for dark mode)
    static const juce::Colour md3BackgroundDark;           // #121318
    static const juce::Colour md3CardFilledDark;           // #1A1B21 (surfaceContainerLow)
    static const juce::Colour md3PrimaryDark;              // #B8C4FF
    static const juce::Colour md3OnSurfaceVariantDark;     // #C6C5D0
    static const juce::Colour md3SecondaryContainerDark;   // #424659
    static const juce::Colour md3OnSecondaryContainerDark; // #DEE1F9
    static const juce::Colour md3SurfaceVariantDark;       // #45464F
    static const juce::Colour md3NavBarDark;               // #1E2A5E (dark navy)
    static const juce::Colour md3OnNavBarDark;             // #FFFFFF

    // MD3 Surface / Outline tokens (light)
    static const juce::Colour md3SurfaceContainer;        // #EFEDF4
    static const juce::Colour md3SurfaceContainerHigh;    // #E9E7EF
    static const juce::Colour md3SurfaceContainerHighest; // #E9E7EF (same as High for light)
    static const juce::Colour md3OnSurface;               // #1A1B21
    static const juce::Colour md3OutlineVariant;          // #C6C5D0

    // MD3 Surface / Outline tokens (dark)
    static const juce::Colour md3SurfaceContainerDark;        // #1F1F25
    static const juce::Colour md3SurfaceContainerHighDark;    // #292A2F
    static const juce::Colour md3SurfaceContainerHighestDark; // #292A2F (same as High for dark)
    static const juce::Colour md3OnSurfaceDark;               // #E3E1E9
    static const juce::Colour md3OutlineVariantDark;          // #45464F

    KiraNastroLookAndFeel();
    ~KiraNastroLookAndFeel() override = default;

    // Dark mode API
    void setDarkMode(bool dark);
    bool getDarkMode() const { return m_isDark; }

    // Instance-level colour accessors (respects isDark state)
    juce::Colour background() const;
    juce::Colour cardFilled() const;
    juce::Colour primary() const;
    juce::Colour onSurfaceVariant() const;
    juce::Colour secondaryContainer() const;
    juce::Colour onSecondaryContainer() const;
    juce::Colour surfaceVariant() const;
    juce::Colour navBar() const;
    juce::Colour onNavBar() const;
    juce::Colour surfaceContainer() const;
    juce::Colour surfaceContainerHigh() const;
    juce::Colour surfaceContainerHighest() const;
    juce::Colour onSurface() const;
    juce::Colour outlineVariant() const;

    // MD3 filled button / text button (auto-detects AlertWindow context)
    void drawButtonBackground(juce::Graphics &g, juce::Button &button, const juce::Colour &backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText(juce::Graphics &g, juce::TextButton &button,
                        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawLabel(juce::Graphics &g, juce::Label &label) override;

    // MD3 popup menu
    void drawPopupMenuBackground(juce::Graphics &g, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area, bool isSeparator, bool isActive,
                           bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String &text,
                           const juce::String &shortcutKeyText, const juce::Drawable *icon,
                           const juce::Colour *textColour) override;
    void getIdealPopupMenuItemSize(const juce::String &text, bool isSeparator, int standardMenuItemHeight,
                                   int &idealWidth, int &idealHeight) override;
    int getPopupMenuBorderSize() override;

    // MD3 dialog (AlertWindow)
    void drawAlertBox(juce::Graphics &, juce::AlertWindow &, const juce::Rectangle<int> &textArea,
                      juce::TextLayout &) override;
    int  getAlertWindowButtonHeight() override;
    juce::Font getAlertWindowTitleFont() override;
    juce::Font getAlertWindowMessageFont() override;

private:
    bool m_isDark = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroLookAndFeel)
};
