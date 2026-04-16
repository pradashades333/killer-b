cmake_minimum_required(VERSION 3.22)
project(KillerBSynth VERSION 1.0.0)

# -----------------------------------------------------------------------
# JUCE — expects a 'JUCE' subdirectory at the project root.
# Clone it with: git submodule add https://github.com/juce-framework/JUCE.git
# -----------------------------------------------------------------------
add_subdirectory(JUCE)

juce_add_binary_data(KillerBAssets
    SOURCES
        "KILLA B.png"
        "GOLD BEE.png"
        "TURNE ME UP.png"
)

juce_add_plugin(KillerBSynth
    COMPANY_NAME            "KillerB"
    PLUGIN_MANUFACTURER_CODE KlrB
    PLUGIN_CODE             Kb01
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
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
