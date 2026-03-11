cmake_minimum_required(VERSION 3.22)
project(KiraNastro VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Third-party dependencies
add_subdirectory(3rdparty/JUCE)

#==============================================================================
# Opus stack (ogg → opus → opusfile), all BSD-3-Clause / compatible with GPLv3
include(FetchContent)

FetchContent_Declare(ogg
    GIT_REPOSITORY https://github.com/xiph/ogg.git
    GIT_TAG        v1.3.5
    GIT_SHALLOW    TRUE
)
FetchContent_Declare(opus
    GIT_REPOSITORY https://github.com/xiph/opus.git
    GIT_TAG        v1.5.2
    GIT_SHALLOW    TRUE
)

# Opusfile options: disable HTTP, examples and docs (we only need the decode API)
set(OP_DISABLE_HTTP     ON CACHE BOOL "" FORCE)
set(OP_DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(OP_DISABLE_DOCS     ON CACHE BOOL "" FORCE)

FetchContent_Declare(opusfile
    GIT_REPOSITORY https://github.com/xiph/opusfile.git
    GIT_TAG        v0.12
    GIT_SHALLOW    TRUE
)

# xiph CMakeLists.txt files may declare cmake_minimum_required < 3.5.
# CMake 3.27+ provides CMAKE_POLICY_VERSION_MINIMUM to relax the check.
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.27")
    set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE INTERNAL "")
endif()

# Build ogg and opus first; then add their build dirs to CMAKE_PREFIX_PATH so
# opusfile's internal find_package(Ogg) / find_package(Opus) calls find them.
FetchContent_MakeAvailable(ogg opus)
FetchContent_GetProperties(ogg)
FetchContent_GetProperties(opus)
list(APPEND CMAKE_PREFIX_PATH "${ogg_BINARY_DIR}" "${opus_BINARY_DIR}")

# opusfile v0.12 has no CMakeLists.txt — populate sources only, then build manually.
FetchContent_GetProperties(opusfile)
if(NOT opusfile_POPULATED)
    FetchContent_Populate(opusfile)
endif()

# The opusfile sources use #ifdef HAVE_CONFIG_H guards around their config include.
# By not defining HAVE_CONFIG_H we skip the autoconf-generated header entirely and
# get safe defaults: float API, no HTTP, no fixed-point — exactly what we want.
add_library(opusfile STATIC
    "${opusfile_SOURCE_DIR}/src/info.c"
    "${opusfile_SOURCE_DIR}/src/internal.c"
    "${opusfile_SOURCE_DIR}/src/opusfile.c"
    "${opusfile_SOURCE_DIR}/src/stream.c"
)

target_include_directories(opusfile
    PUBLIC
        "${opusfile_SOURCE_DIR}/include"
)

target_link_libraries(opusfile
    PUBLIC
        Ogg::ogg
        Opus::opus
)

target_compile_features(opusfile PRIVATE c_std_99)

# Suppress warnings from third-party opusfile sources
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(opusfile PRIVATE -w)
endif()

enable_testing()

# Embed fonts as binary data
juce_add_binary_data(KiraNastroFonts
    HEADER_NAME "BinaryDataFonts.h"
    NAMESPACE "BinaryDataFonts"
    SOURCES
        resources/fonts/SarasaUiJ-Regular.ttf
        resources/fonts/SarasaUiJ-SemiBold.ttf
        resources/fonts/SarasaUiJ-Bold.ttf
        resources/fonts/Lexend-Regular.ttf
        resources/fonts/Lexend-Bold.ttf
)

# Embed branding images as binary data
juce_add_binary_data(KiraNastroImages
    HEADER_NAME "BinaryDataImages.h"
    NAMESPACE   "BinaryDataImages"
    SOURCES
        resources/branding/icon.svg
)

# Plugin target
juce_add_plugin(KiraNastro
    VERSION                     0.1.0
    COMPANY_NAME                "KiraNastro"
    COMPANY_WEBSITE             ""
    COMPANY_EMAIL               ""
    PLUGIN_MANUFACTURER_CODE    KNVT
    PLUGIN_CODE                 Knst
    PRODUCT_NAME                "KiraNastro inst"
    LV2URI                      "urn:kiranastro:KiraNastroInst"
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
        src/KiraNastroProcessor.h
        src/KiraNastroProcessor.cpp
        src/KiraNastroEditor.h
        src/KiraNastroEditor.cpp
        src/audio/BGMPlayer.h
        src/audio/BGMPlayer.cpp
        src/audio/OpusAudioFormat.h
        src/audio/OpusAudioFormat.cpp
        src/data/ReclistParser.h
        src/data/ReclistParser.cpp
        src/data/GuideBGMParser.h
        src/data/GuideBGMParser.cpp
        src/data/LabelExporter.h
        src/data/LabelExporter.cpp
        src/ui/LookAndFeel.h
        src/ui/LookAndFeel.cpp
        src/ui/TimingIndicator.h
        src/ui/TimingIndicator.cpp
        src/ui/PlaybackControls.h
        src/ui/PlaybackControls.cpp
        src/ui/MD3Dialog.h
        src/ui/MD3Dialog.cpp
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
        JUCE_USE_MP3AUDIOFORMAT=1
)

target_include_directories(KiraNastro
    PRIVATE
        src
)

target_link_libraries(KiraNastro
    PRIVATE
        KiraNastroFonts
        KiraNastroImages
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_gui_extra
        juce::juce_animation
        opusfile
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

add_subdirectory(tests)
