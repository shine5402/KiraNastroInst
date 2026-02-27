cmake_minimum_required(VERSION 3.22)
project(KiraNastro VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE submodule
add_subdirectory(JUCE)

enable_testing()

# Embed fonts as binary data
juce_add_binary_data(KiraNastroFonts
    HEADER_NAME "BinaryDataFonts.h"
    NAMESPACE "BinaryDataFonts"
    SOURCES
        resources/fonts/SarasaUiJ-Regular.ttf
        resources/fonts/SarasaUiJ-Bold.ttf
        resources/fonts/Lexend-Regular.ttf
        resources/fonts/Lexend-Bold.ttf
)

# Plugin target
juce_add_plugin(KiraNastro
    VERSION                     0.1.0
    COMPANY_NAME                "KiraNastro"
    COMPANY_WEBSITE             ""
    COMPANY_EMAIL               ""
    PLUGIN_MANUFACTURER_CODE    KNVT
    PLUGIN_CODE                 Knst
    PRODUCT_NAME                "KiraNastro VSTi"
    LV2URI                      "urn:kiranastro:KiraNastroVSTi"
    DESCRIPTION                 "UTAU VCV voicebank recording guide plugin"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    FORMATS                     VST3 AU LV2 Standalone
    VST3_CATEGORIES             "Instrument"
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice
)

target_sources(KiraNastro
    PRIVATE
        src/PluginProcessor.h
        src/PluginProcessor.cpp
        src/PluginEditor.h
        src/PluginEditor.cpp
        src/audio/BGMPlayer.h
        src/audio/BGMPlayer.cpp
        src/data/ReclistParser.h
        src/data/ReclistParser.cpp
        src/data/GuideBGMParser.h
        src/data/GuideBGMParser.cpp
        src/data/LabelExporter.h
        src/data/LabelExporter.cpp
        src/ui/LookAndFeel.h
        src/ui/LookAndFeel.cpp
        src/ui/MainComponent.h
        src/ui/MainComponent.cpp
        src/ui/TimingIndicator.h
        src/ui/TimingIndicator.cpp
        src/utils/Fonts.h
        src/utils/TextEncoding.h
        src/utils/TextEncoding.cpp
)

target_compile_definitions(KiraNastro
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_include_directories(KiraNastro
    PRIVATE
        src
)

target_link_libraries(KiraNastro
    PRIVATE
        KiraNastroFonts
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

add_subdirectory(tests)
