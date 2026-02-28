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
    static const juce::Colour md3BackgroundDark;            // #121318
    static const juce::Colour md3CardFilledDark;            // #1A1B21 (surfaceContainerLow)
    static const juce::Colour md3PrimaryDark;               // #B8C4FF
    static const juce::Colour md3OnSurfaceVariantDark;      // #C6C5D0
    static const juce::Colour md3SecondaryContainerDark;    // #424659
    static const juce::Colour md3OnSecondaryContainerDark;  // #DEE1F9
    static const juce::Colour md3SurfaceVariantDark;        // #45464F
    static const juce::Colour md3NavBarDark;                // #1E2A5E (dark navy)
    static const juce::Colour md3OnNavBarDark;              // #FFFFFF

    KiraNastroLookAndFeel();
    ~KiraNastroLookAndFeel() override = default;

    // Dark mode API
    void setDarkMode(bool dark);
    bool getDarkMode() const { return isDark; }

    // Instance-level colour accessors (respects isDark state)
    juce::Colour background()              const;
    juce::Colour cardFilled()              const;
    juce::Colour primary()                 const;
    juce::Colour onSurfaceVariant()        const;
    juce::Colour secondaryContainer()      const;
    juce::Colour onSecondaryContainer()    const;
    juce::Colour surfaceVariant()          const;
    juce::Colour navBar()                  const;
    juce::Colour onNavBar()                const;

    // MD3 filled button
    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    // MD3 popup menu
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics& g,
                           const juce::Rectangle<int>& area,
                           bool isSeparator,
                           bool isActive,
                           bool isHighlighted,
                           bool isTicked,
                           bool hasSubMenu,
                           const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon,
                           const juce::Colour* textColour) override;
    int getPopupMenuBorderSize() override;

private:
    bool isDark = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KiraNastroLookAndFeel)
};
