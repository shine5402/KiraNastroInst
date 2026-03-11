add_subdirectory(../3rdparty/Catch2 Catch2)

juce_add_console_app(KiraNastro_Tests
    PRODUCT_NAME "KiraNastro Tests"
)

target_sources(KiraNastro_Tests PRIVATE
    TextEncodingTest.cpp
    ReclistParserTest.cpp
    GuideBGMParserTest.cpp
    BGMPlayerTest.cpp
    LabelExporterTest.cpp
    ../src/utils/TextEncoding.cpp
    ../src/data/ReclistParser.cpp
    ../src/data/GuideBGMParser.cpp
    ../src/data/LabelExporter.cpp
    ../src/audio/BGMPlayer.cpp
    ../src/audio/OpusAudioFormat.cpp
)

target_include_directories(KiraNastro_Tests PRIVATE
    ../src
)

target_compile_definitions(KiraNastro_Tests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_MP3AUDIOFORMAT=1
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata"
)

# juce_audio_formats brings in juce_audio_basics and juce_core transitively
target_link_libraries(KiraNastro_Tests PRIVATE
    juce::juce_audio_formats
    opusfile
    Catch2::Catch2WithMain
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/Catch2/extras")
include(Catch)
# Exit code 4 = all tests skipped; tell CTest to treat this as "skipped" not "failed"
catch_discover_tests(KiraNastro_Tests PROPERTIES SKIP_RETURN_CODE 4)
