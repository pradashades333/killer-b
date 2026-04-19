cmake_minimum_required(VERSION 3.22)
project(KillerBSynth VERSION 1.0.0)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES    "arm64;x86_64" CACHE STRING "Universal binary" FORCE)
endif()

# -----------------------------------------------------------------------
# JUCE — expects a 'JUCE' subdirectory at the project root.
# Clone it with: git submodule add https://github.com/juce-framework/JUCE.git
# -----------------------------------------------------------------------
add_subdirectory(JUCE)

option(KILLERB_COPY_AFTER_BUILD "Copy plugin to system folder after build" ON)

if(WIN32)
    set(_VST3_DIR "C:/Program Files/Common Files/VST3")
    set(_AU_DIR   "")
elseif(APPLE)
    set(_VST3_DIR "/Library/Audio/Plug-Ins/VST3")
    set(_AU_DIR   "/Library/Audio/Plug-Ins/Components")
endif()

juce_add_binary_data(KillerBAssets
    SOURCES
        "KILLA B.png"
        "GOLD BEE.png"
        "TURNE ME UP.png"
        "knob 2.png"
)

juce_add_plugin(KillerBSynth
    COMPANY_NAME            "KillerB"
    BUNDLE_ID               "com.killerb.killerbsynth"
    PLUGIN_MANUFACTURER_CODE KlrB
    PLUGIN_CODE             Kb01
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${KILLERB_COPY_AFTER_BUILD}
    VST3_COPY_DIR           "${_VST3_DIR}"
    AU_COPY_DIR             "${_AU_DIR}"
    FORMATS                 VST3 AU Standalone
    PRODUCT_NAME            "Killer B Synth"
    DESCRIPTION             "Polyphonic Trap Synthesizer"
)

# Generates JuceHeader.h and adds its directory to the target's include path.
# Required in JUCE 7+ CMake builds — without this, #include <JuceHeader.h> fails.
juce_generate_juce_header(KillerBSynth)

target_sources(KillerBSynth
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/KillaBLookAndFeel.h
)

target_compile_definitions(KillerBSynth
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(KillerBSynth
    PRIVATE
        KillerBAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# C++17 required for std::make_unique and structured bindings
target_compile_features(KillerBSynth PUBLIC cxx_std_17)
