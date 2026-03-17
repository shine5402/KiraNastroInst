// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Material Symbols Outlined, 24dp, wght=400, FILL=0
// Source SVGs: resources/icons/*.svg (fill="#1f1f1f" — replaced on load via replaceColour)
namespace Icons
{
static const char *const menuSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M120-240v-80h720v80H120Zm0-200v-80h720v80H120Zm0-200v-80h720v80H120Z"/></svg>)SVG";

static const char *const arrowRightSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M383-480 200-664l56-56 240 240-240 240-56-56 183-184Zm264 0L464-664l56-56 240 240-240 240-56-56 183-184Z"/></svg>)SVG";

static const char *const percentSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M300-520q-58 0-99-41t-41-99q0-58 41-99t99-41q58 0 99 41t41 99q0 58-41 99t-99 41Zm0-80q25 0 42.5-17.5T360-660q0-25-17.5-42.5T300-720q-25 0-42.5 17.5T240-660q0 25 17.5 42.5T300-600Zm360 440q-58 0-99-41t-41-99q0-58 41-99t99-41q58 0 99 41t41 99q0 58-41 99t-99 41Zm42.5-97.5Q720-275 720-300t-17.5-42.5Q685-360 660-360t-42.5 17.5Q600-325 600-300t17.5 42.5Q635-240 660-240t42.5-17.5ZM216-160l-56-56 584-584 56 56-584 584Z"/></svg>)SVG";

static const char *const checkSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M382-240 154-468l57-57 171 171 367-367 57 57-424 424Z"/></svg>)SVG";

static const char *const chevronDownSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M480-344 240-584l56-56 184 184 184-184 56 56-240 240Z"/></svg>)SVG";

static const char *const folderOpenSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M160-160q-33 0-56.5-23.5T80-240v-480q0-33 23.5-56.5T160-800h240l80 80h320q33 0 56.5 23.5T880-640H447l-80-80H160v480l96-320h684L837-160H160Zm84-80h516l72-240H316l-72 240Zm0 0 72-240-72 240Zm-84-400v-80 80Z"/></svg>)SVG";

static const char *const closeSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="m256-200-56-56 224-224-224-224 56-56 224 224 224-224 56 56-224 224 224 224-56 56-224-224-224 224Z"/></svg>)SVG";

static const char *const playArrowSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M320-200v-560l440 280-440 280Z"/></svg>)SVG";

static const char *const stopSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M240-240v-480h480v480H240Z"/></svg>)SVG";

static const char *const skipNextSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M660-240v-480h80v480h-80Zm-440 0v-480l360 240-360 240Z"/></svg>)SVG";

static const char *const skipPreviousSvg =
    R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M220-240v-480h80v480h-80Zm520 0L380-480l360-240v480Z"/></svg>)SVG";

// Load SVG and replace its #1f1f1f fill with the given tint colour
inline std::unique_ptr<juce::Drawable> load(const char *svgText, juce::Colour tint = juce::Colour(0xFF1F1F1F))
{
    auto xml = juce::XmlDocument::parse(juce::String::fromUTF8(svgText));
    if (!xml)
        return nullptr;
    auto d = juce::Drawable::createFromSVG(*xml);
    if (d)
        d->replaceColour(juce::Colour(0xFF1F1F1F), tint);
    return d;
}
} // namespace Icons
