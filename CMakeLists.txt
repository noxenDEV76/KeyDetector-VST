cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0091 NEW)
# Runtime Visual C++ lié statiquement : aucune dépendance à installer sur le PC.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(KeyDetector VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

# Windows : VST3 uniquement.
if(WIN32)
    set(KD_FORMATS VST3)
else()
    set(KD_FORMATS VST3 Standalone)
    if(APPLE)
        list(APPEND KD_FORMATS AU)
    endif()
endif()

juce_add_plugin(KeyDetector
    COMPANY_NAME "KeyDetectorLab"
    PRODUCT_NAME "Key Detector"
    PLUGIN_MANUFACTURER_CODE Kdlb
    PLUGIN_CODE Kdet
    FORMATS ${KD_FORMATS}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx Analyzer
    VST3_AUTO_MANIFEST FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(KeyDetector PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(KeyDetector PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(KeyDetector
    PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_warning_flags)

if(NOT WIN32)
    target_link_libraries(KeyDetector PUBLIC juce::juce_recommended_lto_flags)
endif()
