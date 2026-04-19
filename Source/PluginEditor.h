#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "KillaBLookAndFeel.h"

class SegmentedMeterBar : public juce::Component
{
public:
    SegmentedMeterBar() = default;
    void setLevel (float level) noexcept { currentLevel = juce::jlimit (0.0f, 1.0f, level); }
    void paint (juce::Graphics& g) override;

private:
    float currentLevel = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SegmentedMeterBar)
};

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay() = default;
    void setPhase (float p) noexcept { phase = p; }
    void setDriveAmount (float d) noexcept { driveAmount = juce::jlimit (0.0f, 1.0f, d); }
    void paint (juce::Graphics& g) override;

private:
    float phase = 0.0f;
    float driveAmount = 0.5f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};

class KillaBKeyboard : public juce::MidiKeyboardComponent
{
public:
    explicit KillaBKeyboard (juce::MidiKeyboardState& state);
    void drawWhiteNote (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                        bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override;
    void drawBlackNote (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                        bool isDown, bool isOver, juce::Colour noteFillColour) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KillaBKeyboard)
};

class KillaBEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit KillaBEditor (KillerBProcessor&);
    ~KillaBEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& text, const juce::String& colourTag);
    void setupVertSlider (juce::Slider& slider, juce::Label& label, const juce::String& text);
    void setupCombo (juce::ComboBox& box, juce::Label& label, const juce::String& text);
    void setupPassiveKnob (juce::Slider& knob, juce::Label& label, const juce::String& text);
    void populateFactoryPresets();
    void applyFactoryPreset (int presetIndex);

    void paintBackground (juce::Graphics& g);
    void paintPanel (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title);
    void paintHeaderText (juce::Graphics& g);
    void paintLogo (juce::Graphics& g);
    void paintCenterBrand (juce::Graphics& g);
    void paintOrb (juce::Graphics& g, juce::Rectangle<float> area);
    void paintDistortionMeters (juce::Graphics& g, juce::Rectangle<int> bounds);
    void paintMascotPlaceholder (juce::Graphics& g, juce::Rectangle<int> bounds);
    void paintAdsrGuides (juce::Graphics& g, juce::Rectangle<int> bounds);
    void paintEqGuides (juce::Graphics& g, juce::Rectangle<int> bounds);
    void paintFxLabels (juce::Graphics& g);

    KillerBProcessor& processorRef;
    KillaBLookAndFeel laf;
    KillaBKeyboard keyboard;
    juce::Image topLeftLogoImage;
    juce::Image centerBrandImage;
    juce::Image beeImage;
    bool isApplyingFactoryPreset = false;

    juce::ComboBox presetCombo, catCombo;
    juce::Label presetTitleLabel, modeTitleLabel;

    std::array<juce::Slider, 4> fxAmtKnobs, fxMixKnobs, fxDetailKnobs;
    std::array<juce::Label, 4> fxAmtLabels, fxMixLabels, fxDetailLabels;
    std::array<juce::Rectangle<int>, 4> fxCardAreas;
    juce::ComboBox reverbPresetBox;
    juce::Label reverbPresetLabel;

    juce::ComboBox filterTypeBox, filterSlopeBox, voicesBox;
    juce::Label filterTypeLabel, filterSlopeLabel, voicesLabel;

    juce::ComboBox osc1TypeBox, osc2TypeBox;
    juce::Slider oscMixKnob;
    juce::Label osc1Label, osc2Label, oscMixLabel;

    juce::Slider masterGainKnob, glideKnob, cutoffKnob;
    juce::Label masterGainLabel, glideLabel, cutoffLabel;

    juce::Slider driveKnob;
    juce::Label driveLabel;
    WaveformDisplay waveDisplay;

    juce::Slider compThreshKnob, compRatioKnob, compAttackKnob, compReleaseKnob, compMixKnob;
    juce::Label compThreshLabel, compRatioLabel, compAttackLabel, compReleaseLabel, compMixLabel;

    juce::Slider eqLowKnob, eqMidKnob, eqHighKnob;
    juce::Label eqLowLabel, eqMidLabel, eqHighLabel;
    SegmentedMeterBar meterBarLow, meterBarMid, meterBarHigh;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;

    juce::ComboBox lfoRoutingBox, lfoTypeBox;
    juce::Label lfoRoutingLabel, lfoTypeLabel;
    juce::Slider lfoRateKnob, lfoDepthKnob;
    juce::Label lfoRateLabel, lfoDepthLabel;

    float displayLow = 0.0f;
    float displayMid = 0.0f;
    float displayHigh = 0.0f;
    float wavePhase = 0.0f;
    float orbitAngle = 0.0f;

    juce::Rectangle<int> headerArea, topArea, bottomArea, keyboardArea;
    juce::Rectangle<int> leftArea, centerArea, rightArea;
    juce::Rectangle<int> distArea, compArea, eqArea, adsrArea, lfoArea, mascotArea;
    juce::Rectangle<int> orbArea, waveArea;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<CA> osc1TypeAttach, osc2TypeAttach;
    std::unique_ptr<SA> oscMixAttach, masterGainAttach, driveAttach;
    std::unique_ptr<SA> compThreshAttach, compRatioAttach, compAttackAttach, compReleaseAttach;
    std::array<std::unique_ptr<SA>, 4> fxAmtAttaches, fxMixAttaches;
    std::unique_ptr<SA> eqLowAttach, eqMidAttach, eqHighAttach;
    std::unique_ptr<SA> attackAttach, decayAttach, sustainAttach, releaseAttach;
    std::unique_ptr<SA> glideAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KillaBEditor)
};
