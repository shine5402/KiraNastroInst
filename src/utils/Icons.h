#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Material Symbols Outlined, 24dp, wght=400, FILL=0
// Source SVGs: resources/icons/*.svg (fill="#1f1f1f" — replaced on load via replaceColour)
namespace Icons
{
    static const char* const menuSvg = R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M120-240v-80h720v80H120Zm0-200v-80h720v80H120Zm0-200v-80h720v80H120Z"/></svg>)SVG";

    static const char* const arrowRightSvg = R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M383-480 200-664l56-56 240 240-240 240-56-56 183-184Zm264 0L464-664l56-56 240 240-240 240-56-56 183-184Z"/></svg>)SVG";

    static const char* const percentSvg = R"SVG(<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#1f1f1f"><path d="M300-520q-58 0-99-41t-41-99q0-58 41-99t99-41q58 0 99 41t41 99q0 58-41 99t-99 41Zm0-80q25 0 42.5-17.5T360-660q0-25-17.5-42.5T300-720q-25 0-42.5 17.5T240-660q0 25 17.5 42.5T300-600Zm360 440q-58 0-99-41t-41-99q0-58 41-99t99-41q58 0 99 41t41 99q0 58-41 99t-99 41Zm42.5-97.5Q720-275 720-300t-17.5-42.5Q685-360 660-360t-42.5 17.5Q600-325 600-300t17.5 42.5Q635-240 660-240t42.5-17.5ZM216-160l-56-56 584-584 56 56-584 584Z"/></svg>)SVG";

    // Load SVG and replace its #1f1f1f fill with the given tint colour
    inline std::unique_ptr<juce::Drawable> load(const char* svgText,
                                                 juce::Colour tint = juce::Colour(0xFF1F1F1F))
    {
        auto xml = juce::XmlDocument::parse(juce::String::fromUTF8(svgText));
        if (!xml) return nullptr;
        auto d = juce::Drawable::createFromSVG(*xml);
        if (d) d->replaceColour(juce::Colour(0xFF1F1F1F), tint);
        return d;
    }
}
