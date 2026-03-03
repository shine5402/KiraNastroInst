// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_graphics/juce_graphics.h>

#include "BinaryDataFonts.h"

namespace Fonts
{
inline juce::Typeface::Ptr getSarasaRegular()
{
    static juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryDataFonts::SarasaUiJRegular_ttf, BinaryDataFonts::SarasaUiJRegular_ttfSize);
    return typeface;
}

inline juce::Typeface::Ptr getSarasaSemiBold()
{
    static juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryDataFonts::SarasaUiJSemiBold_ttf, BinaryDataFonts::SarasaUiJSemiBold_ttfSize);
    return typeface;
}

inline juce::Typeface::Ptr getSarasaBold()
{
    static juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryDataFonts::SarasaUiJBold_ttf, BinaryDataFonts::SarasaUiJBold_ttfSize);
    return typeface;
}

inline juce::Typeface::Ptr getLexendRegular()
{
    static juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryDataFonts::LexendRegular_ttf, BinaryDataFonts::LexendRegular_ttfSize);
    return typeface;
}

inline juce::Typeface::Ptr getLexendBold()
{
    static juce::Typeface::Ptr typeface =
        juce::Typeface::createSystemTypefaceFor(BinaryDataFonts::LexendBold_ttf, BinaryDataFonts::LexendBold_ttfSize);
    return typeface;
}
} // namespace Fonts
