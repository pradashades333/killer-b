#include "PluginEditor.h"

namespace FXMeta
{
    static const char* amtIds[] = { "CHORUS_AMOUNT", "DELAY_AMOUNT", "PHASER_AMOUNT", "REVERB_AMOUNT" };
    static const char* mixIds[] = { "CHORUS_MIX", "DELAY_MIX", "PHASER_MIX", "REVERB_MIX" };
    static const char* names[] = { "CHORUS FX", "DELAY FX", "PHASER FX", "REVERB FX" };
    static const char* amtLabels[] = { "rate", "time", "freq", "size" };
    static const char* detailLabels[] = { "depth", "feedback", "depth", "damping" };
    static const char* mixLabels[] = { "mix", "mix", "mix", "mix" };
}

void SegmentedMeterBar::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    constexpr int numSegs = 22;
    constexpr float gap = 2.0f;
    const float segH = (bounds.getHeight() - gap * (numSegs - 1)) / (float) numSegs;

    for (int i = 0; i < numSegs; ++i)
    {
        const float norm = (float) i / (float) (numSegs - 1);
        const float y = bounds.getBottom() - (float) (i + 1) * segH - (float) i * gap;
        juce::Rectangle<float> seg (bounds.getX(), y, bounds.getWidth(), segH);
        const bool lit = norm <= currentLevel;
        g.setColour (lit ? KBColours::goldLight.withAlpha (0.92f) : KBColours::gold.withAlpha (0.16f));
        g.fillRect (seg);
    }
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    juce::ColourGradient bg (juce::Colour (0xff171722), b.getX(), b.getY(),
                             juce::Colour (0xff101017), b.getRight(), b.getBottom(), true);
    g.setGradientFill (bg);
    g.fillRoundedRectangle (b, 2.0f);
    g.setColour (KBColours::gold.withAlpha (0.14f));
    g.drawRect (b.toNearestInt(), 1);

    juce::Path wave;
    const int points = juce::jmax (80, (int) b.getWidth());
    const float drive = 1.0f + driveAmount * 9.0f;
    const float normFactor = (drive > 1.0f) ? (1.0f / std::tanh (drive)) : 1.0f;
    for (int i = 0; i <= points; ++i)
    {
        const float t = (float) i / (float) points;
        const float x = b.getX() + t * b.getWidth();
        const float s = std::sin (t * juce::MathConstants<float>::twoPi * 1.8f + phase);
        const float y = b.getCentreY() - std::tanh (s * drive) * normFactor * b.getHeight() * 0.30f;
        if (i == 0) wave.startNewSubPath (x, y);
        else wave.lineTo (x, y);
    }

    for (int i = 0; i < 8; ++i)
    {
        juce::Path shifted (wave);
        shifted.applyTransform (juce::AffineTransform::translation (0.0f, (float) i * 4.0f));
        g.setColour (juce::Colour (0xfff0b263).withAlpha (0.08f + 0.03f * (float) (7 - i)));
        g.strokePath (shifted, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    g.setColour (juce::Colour (0xffffc676).withAlpha (0.75f));
    g.strokePath (wave, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

KillaBKeyboard::KillaBKeyboard (juce::MidiKeyboardState& state)
    : juce::MidiKeyboardComponent (state, juce::MidiKeyboardComponent::horizontalKeyboard)
{
}

void KillaBKeyboard::drawWhiteNote (int, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver,
                                    juce::Colour lineColour, juce::Colour)
{
    auto fill = isDown ? juce::Colour (0xffe7d7bc) : (isOver ? juce::Colour (0xfff7f3ee) : juce::Colour (0xfff5f4f2));
    g.setColour (fill);
    g.fillRect (area);
    g.setColour (lineColour.withAlpha (0.45f));
    g.drawRect (area, 1.0f);
}

void KillaBKeyboard::drawBlackNote (int, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver,
                                    juce::Colour)
{
    auto fill = isDown ? juce::Colour (0xff1d1d1f) : (isOver ? juce::Colour (0xff2a2a2d) : juce::Colours::black);
    g.setColour (fill);
    g.fillRect (area.reduced (1.0f, 0.0f));
}

KillaBEditor::KillaBEditor (KillerBProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), keyboard (p.keyboardState)
{
    auto loadBinaryImageIfPresent = [] (const char* resourceName)
    {
        int dataSize = 0;
        if (auto* data = BinaryData::getNamedResource (resourceName, dataSize))
            return juce::ImageFileFormat::loadFrom (data, (size_t) dataSize);

        return juce::Image();
    };

    setLookAndFeel (&laf);
    setSize (1110, 1110);

    topLeftLogoImage = loadBinaryImageIfPresent ("KILLA B.png");
    centerBrandImage = loadBinaryImageIfPresent ("TURNE ME UP.png");
    beeImage = loadBinaryImageIfPresent ("GOLD BEE.png");

    setupCombo (presetCombo, presetTitleLabel, "PRESETS");
    presetCombo.addItem ("Bass - Dark Moog", 1);
    presetCombo.addItem ("Init Patch", 2);
    presetCombo.addItem ("Hard Lead", 3);
    presetCombo.setSelectedId (1, juce::dontSendNotification);

    setupCombo (catCombo, modeTitleLabel, "SYNTHMODE");
    catCombo.addItem ("Poly", 1);
    catCombo.addItem ("Mono", 2);
    catCombo.addItem ("Legato", 3);
    catCombo.setSelectedId (1, juce::dontSendNotification);

    for (int i = 0; i < 4; ++i)
    {
        setupKnob (fxAmtKnobs[i], fxAmtLabels[i], FXMeta::amtLabels[i], "gold");
        setupKnob (fxMixKnobs[i], fxMixLabels[i], FXMeta::mixLabels[i], "gold");
        setupPassiveKnob (fxDetailKnobs[i], fxDetailLabels[i], FXMeta::detailLabels[i]);
        fxAmtKnobs[i].setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        fxMixKnobs[i].setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        fxDetailKnobs[i].setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        fxAmtAttaches[i] = std::make_unique<SA> (processorRef.apvts, FXMeta::amtIds[i], fxAmtKnobs[i]);
        fxMixAttaches[i] = std::make_unique<SA> (processorRef.apvts, FXMeta::mixIds[i], fxMixKnobs[i]);
    }

    setupCombo (reverbPresetBox, reverbPresetLabel, "");
    reverbPresetBox.addItem ("Church", 1);
    reverbPresetBox.addItem ("Hall", 2);
    reverbPresetBox.addItem ("Plate", 3);
    reverbPresetBox.setSelectedId (1, juce::dontSendNotification);

    setupCombo (filterTypeBox, filterTypeLabel, "Filter type");
    filterTypeBox.addItem ("None", 1);
    filterTypeBox.addItem ("Low-pass", 2);
    filterTypeBox.addItem ("High-pass", 3);
    filterTypeBox.setSelectedId (1, juce::dontSendNotification);

    setupCombo (filterSlopeBox, filterSlopeLabel, "Filter slope");
    filterSlopeBox.addItem ("12 dB/oct", 1);
    filterSlopeBox.addItem ("24 dB/oct", 2);
    filterSlopeBox.setSelectedId (1, juce::dontSendNotification);

    setupCombo (voicesBox, voicesLabel, "VOICES");
    voicesBox.addItem ("32", 1);
    voicesBox.addItem ("16", 2);
    voicesBox.addItem ("8", 3);
    voicesBox.setSelectedId (1, juce::dontSendNotification);

    osc1TypeBox.addItem ("Sine", 1);
    osc1TypeBox.addItem ("Square", 2);
    osc1TypeBox.addItem ("Saw", 3);
    osc2TypeBox.addItem ("Sine", 1);
    osc2TypeBox.addItem ("Square", 2);
    osc2TypeBox.addItem ("Saw", 3);
    osc1TypeAttach = std::make_unique<CA> (processorRef.apvts, "osc1type", osc1TypeBox);
    osc2TypeAttach = std::make_unique<CA> (processorRef.apvts, "osc2type", osc2TypeBox);
    oscMixAttach = std::make_unique<SA> (processorRef.apvts, "oscmix", oscMixKnob);

    setupKnob (masterGainKnob, masterGainLabel, "GAIN", "gold");
    masterGainAttach = std::make_unique<SA> (processorRef.apvts, "MASTER_GAIN", masterGainKnob);

    setupKnob (glideKnob, glideLabel, "glide", "gold");
    glideAttach = std::make_unique<SA> (processorRef.apvts, "glide", glideKnob);

    setupPassiveKnob (cutoffKnob, cutoffLabel, "filt. cut off");

    setupKnob (driveKnob, driveLabel, "dist. gain", "orange");
    driveAttach = std::make_unique<SA> (processorRef.apvts, "drive", driveKnob);
    addAndMakeVisible (waveDisplay);

    setupKnob (compThreshKnob, compThreshLabel, "threshold", "gold");
    compThreshAttach = std::make_unique<SA> (processorRef.apvts, "COMP_THRESHOLD", compThreshKnob);
    setupKnob (compRatioKnob, compRatioLabel, "ratio", "gold");
    compRatioAttach = std::make_unique<SA> (processorRef.apvts, "COMP_RATIO", compRatioKnob);
    setupKnob (compAttackKnob, compAttackLabel, "attack", "gold");
    compAttackAttach = std::make_unique<SA> (processorRef.apvts, "COMP_ATTACK", compAttackKnob);
    setupKnob (compReleaseKnob, compReleaseLabel, "release", "gold");
    compReleaseAttach = std::make_unique<SA> (processorRef.apvts, "COMP_RELEASE", compReleaseKnob);
    setupPassiveKnob (compMixKnob, compMixLabel, "mix");

    setupKnob (eqLowKnob, eqLowLabel, "gain", "gold");
    eqLowAttach = std::make_unique<SA> (processorRef.apvts, "LOW_GAIN", eqLowKnob);
    setupKnob (eqMidKnob, eqMidLabel, "gain", "gold");
    eqMidAttach = std::make_unique<SA> (processorRef.apvts, "MID_GAIN", eqMidKnob);
    setupKnob (eqHighKnob, eqHighLabel, "gain", "gold");
    eqHighAttach = std::make_unique<SA> (processorRef.apvts, "HIGH_GAIN", eqHighKnob);
    addAndMakeVisible (meterBarLow);
    addAndMakeVisible (meterBarMid);
    addAndMakeVisible (meterBarHigh);

    setupVertSlider (attackSlider, attackLabel, "Attack");
    attackAttach = std::make_unique<SA> (processorRef.apvts, "attack", attackSlider);
    setupVertSlider (decaySlider, decayLabel, "Decay");
    decayAttach = std::make_unique<SA> (processorRef.apvts, "decay", decaySlider);
    setupVertSlider (sustainSlider, sustainLabel, "Sustain");
    sustainAttach = std::make_unique<SA> (processorRef.apvts, "sustain", sustainSlider);
    setupVertSlider (releaseSlider, releaseLabel, "Release");
    releaseAttach = std::make_unique<SA> (processorRef.apvts, "release", releaseSlider);

    setupCombo (lfoRoutingBox, lfoRoutingLabel, "LFO Routing");
    lfoRoutingBox.addItem ("None", 1);
    lfoRoutingBox.addItem ("Pitch", 2);
    lfoRoutingBox.addItem ("Filter", 3);
    lfoRoutingBox.setSelectedId (1, juce::dontSendNotification);

    setupCombo (lfoTypeBox, lfoTypeLabel, "LFO Type");
    lfoTypeBox.addItem ("Sin", 1);
    lfoTypeBox.addItem ("Tri", 2);
    lfoTypeBox.addItem ("Square", 3);
    lfoTypeBox.setSelectedId (1, juce::dontSendNotification);

    setupPassiveKnob (lfoRateKnob, lfoRateLabel, "lfo rate");
    setupPassiveKnob (lfoDepthKnob, lfoDepthLabel, "lfo depth");

    keyboard.setKeyWidth (19.0f);
    keyboard.setAvailableRange (24, 96);
    addAndMakeVisible (keyboard);

    startTimerHz (30);
}

KillaBEditor::~KillaBEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void KillaBEditor::setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& text, const juce::String& colourTag)
{
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    knob.getProperties().set ("kbColour", colourTag);
    addAndMakeVisible (knob);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (juce::FontOptions ("Segoe UI", 10.0f, juce::Font::plain)).withExtraKerningFactor (0.08f));
    label.setColour (juce::Label::textColourId, KBColours::textGold);
    addAndMakeVisible (label);
}

void KillaBEditor::setupVertSlider (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.getProperties().set ("kbColour", "gold");
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (juce::FontOptions ("Segoe UI", 10.0f, juce::Font::plain)));
    label.setColour (juce::Label::textColourId, KBColours::textGold);
    addAndMakeVisible (label);
}

void KillaBEditor::setupCombo (juce::ComboBox& box, juce::Label& label, const juce::String& text)
{
    addAndMakeVisible (box);
    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::Font (juce::FontOptions ("Segoe UI", 11.0f, juce::Font::bold)).withExtraKerningFactor (0.22f));
    label.setColour (juce::Label::textColourId, KBColours::textGold);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void KillaBEditor::setupPassiveKnob (juce::Slider& knob, juce::Label& label, const juce::String& text)
{
    knob.setRange (0.0, 1.0, 0.001);
    knob.setValue (0.5, juce::dontSendNotification);
    setupKnob (knob, label, text, "gold");
}

void KillaBEditor::timerCallback()
{
    constexpr float decayPerTick = 0.82f;
    auto toNorm = [] (float rmsLinear)
    {
        if (rmsLinear <= 0.0f)
            return 0.0f;
        const float db = juce::Decibels::gainToDecibels (rmsLinear, -60.0f);
        return juce::jlimit (0.0f, 1.0f, juce::jmap (db, -60.0f, 0.0f, 0.0f, 1.0f));
    };

    displayLow = juce::jmax (toNorm (processorRef.levelLow.load (std::memory_order_relaxed)), displayLow * decayPerTick);
    displayMid = juce::jmax (toNorm (processorRef.levelMid.load (std::memory_order_relaxed)), displayMid * decayPerTick);
    displayHigh = juce::jmax (toNorm (processorRef.levelHigh.load (std::memory_order_relaxed)), displayHigh * decayPerTick);

    meterBarLow.setLevel (displayLow);
    meterBarMid.setLevel (displayMid);
    meterBarHigh.setLevel (displayHigh);
    meterBarLow.repaint();
    meterBarMid.repaint();
    meterBarHigh.repaint();

    wavePhase += 0.05f;
    orbitAngle += 0.014f;

    const float driveRaw = processorRef.apvts.getRawParameterValue ("drive")->load();
    waveDisplay.setPhase (wavePhase);
    waveDisplay.setDriveAmount (juce::jmap (driveRaw, 1.0f, 20.0f, 0.0f, 1.0f));
    waveDisplay.repaint();

    repaint();
}

void KillaBEditor::paintBackground (juce::Graphics& g)
{
    KillaBLookAndFeel::drawPluginBackground (g, getWidth(), getHeight());
    g.setColour (KBColours::gold.withAlpha (0.85f));
    g.fillRect (0, headerArea.getBottom() - 1, getWidth(), 1);
    g.setColour (juce::Colours::white.withAlpha (0.035f));
    for (int y = 0; y < getHeight(); y += 2)
        g.fillRect ((y * 13) % juce::jmax (1, getWidth()), y, 1, 1);
}

void KillaBEditor::paintPanel (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title)
{
    g.setColour (juce::Colours::transparentBlack);
    g.fillRect (bounds);
    g.setColour (KBColours::gold.withAlpha (0.72f));
    g.drawRect (bounds, 1);
    g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 10.5f, juce::Font::bold)).withExtraKerningFactor (0.28f));
    g.setColour (KBColours::textGold);
    g.drawText (title, bounds.getX() + 18, bounds.getY() + 20, bounds.getWidth() - 36, 18, juce::Justification::centredLeft);
}

void KillaBEditor::paintLogo (juce::Graphics& g)
{
    if (topLeftLogoImage.isValid())
    {
        g.setOpacity (0.98f);
        g.drawImageWithin (topLeftLogoImage, 12, 8, 180, 70,
                           juce::RectanglePlacement::xLeft | juce::RectanglePlacement::centred,
                           false);
        g.setOpacity (1.0f);
        return;
    }

    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions ("Arial", 40.0f, juce::Font::bold | juce::Font::italic)));
    g.drawText ("Killa B", 14, 12, 160, 56, juce::Justification::centredLeft);
}

void KillaBEditor::paintHeaderText (juce::Graphics& g)
{
    paintLogo (g);
}

void KillaBEditor::paintCenterBrand (juce::Graphics& g)
{
    auto brandBounds = juce::Rectangle<int> (centerArea.getX() + 82, centerArea.getY() + 74, 230, 62);

    if (centerBrandImage.isValid())
    {
        g.setOpacity (0.98f);
        g.drawImageWithin (centerBrandImage,
                           brandBounds.getX(), brandBounds.getY(), brandBounds.getWidth(), brandBounds.getHeight(),
                           juce::RectanglePlacement::centred,
                           false);
        g.setOpacity (1.0f);
        return;
    }

    g.setColour (juce::Colours::black.withAlpha (0.36f));
    g.fillRect (brandBounds);
    g.setFont (juce::Font (juce::FontOptions ("Arial", 50.0f, juce::Font::bold)));
    g.setColour (juce::Colours::white);
    g.drawText ("TURNMEUP", brandBounds, juce::Justification::centred);
}

void KillaBEditor::paintOrb (juce::Graphics& g, juce::Rectangle<float> area)
{
    auto orb = area.withSizeKeepingCentre (juce::jmin (area.getWidth(), area.getHeight()) * 0.62f,
                                           juce::jmin (area.getWidth(), area.getHeight()) * 0.62f);

    for (int i = 5; i > 0; --i)
    {
        auto glow = orb.expanded ((float) i * 14.0f);
        g.setColour (KBColours::gold.withAlpha (0.06f / (float) i));
        g.fillEllipse (glow);
    }

    const auto center = orb.getCentre();
    for (int i = 0; i < 3; ++i)
    {
        juce::Path ring;
        const float rx = orb.getWidth() * (0.73f + 0.14f * (float) i);
        const float ry = orb.getHeight() * (0.52f + 0.12f * (float) i);
        ring.addEllipse (center.x - rx * 0.5f, center.y - ry * 0.5f, rx, ry);
        ring.applyTransform (juce::AffineTransform::rotation (0.35f + 0.24f * (float) i, center.x, center.y));
        g.setColour (KBColours::gold.withAlpha (0.35f - 0.08f * (float) i));
        g.strokePath (ring, juce::PathStrokeType (1.3f));
    }

    juce::Random rng (1729);
    g.setColour (KBColours::goldLight.withAlpha (0.35f));
    for (int i = 0; i < 180; ++i)
    {
        const float a = rng.nextFloat() * juce::MathConstants<float>::twoPi;
        const float r = rng.nextFloat() * orb.getWidth() * 0.46f;
        const float px = center.x + std::cos (a + orbitAngle) * r;
        const float py = center.y + std::sin (a + orbitAngle) * r * 0.9f;
        g.fillEllipse (px, py, 1.7f, 1.7f);
    }

    juce::ColourGradient sphere (KBColours::goldLight, orb.getX() + orb.getWidth() * 0.35f, orb.getY() + orb.getHeight() * 0.18f,
                                 KBColours::goldDark, orb.getRight(), orb.getBottom(), true);
    sphere.addColour (0.55, KBColours::gold);
    g.setGradientFill (sphere);
    g.fillEllipse (orb);
    g.setColour (juce::Colours::white.withAlpha (0.38f));
    g.fillEllipse (orb.getX() + orb.getWidth() * 0.18f, orb.getY() + orb.getHeight() * 0.12f, orb.getWidth() * 0.30f, orb.getHeight() * 0.18f);
}

void KillaBEditor::paintDistortionMeters (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    const char* labels[] = { "in", "out", "filter freq" };
    for (int i = 0; i < 3; ++i)
    {
        const float cx = (float) bounds.getRight() - 38.0f;
        const float cy = (float) bounds.getY() + 26.0f + (float) i * 38.0f;
        g.setColour (KBColours::gold.withAlpha (0.35f));
        g.drawEllipse (cx - 9.0f, cy - 9.0f, 18.0f, 18.0f, 1.0f);
        g.setColour (KBColours::goldLight.withAlpha (0.8f));
        g.drawEllipse (cx - 6.0f, cy - 6.0f, 12.0f, 12.0f, 2.0f);
        g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 10.0f, juce::Font::plain)));
        g.setColour (KBColours::textGold);
        g.drawText (labels[i], (int) cx - 24, (int) cy + 12, 50, 14, juce::Justification::centred);
    }
}

void KillaBEditor::paintMascotPlaceholder (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto innerBounds = bounds.reduced (8, 18);

    if (beeImage.isValid())
    {
        g.setOpacity (0.98f);
        g.drawImageWithin (beeImage,
                           innerBounds.getX(), innerBounds.getY(), innerBounds.getWidth(), innerBounds.getHeight(),
                           juce::RectanglePlacement::centred,
                           true);
        g.setOpacity (1.0f);
        return;
    }

    g.setColour (KBColours::gold.withAlpha (0.22f));
    g.drawRoundedRectangle (bounds.toFloat().reduced (10.0f, 18.0f), 16.0f, 1.0f);
    g.setColour (KBColours::textGold.withAlpha (0.55f));
    g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 13.0f, juce::Font::bold)).withExtraKerningFactor (0.18f));
    g.drawText ("MASCOT LATER", bounds, juce::Justification::centred);
}

void KillaBEditor::paintAdsrGuides (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 12.0f, juce::Font::bold)).withExtraKerningFactor (0.18f));
    g.setColour (KBColours::textGold);
    g.drawText ("ADSR", bounds.removeFromTop (26), juce::Justification::centred);

    auto body = bounds.reduced (14, 18);
    auto columns = body.getWidth() / 4;
    for (int i = 0; i < 4; ++i)
    {
        auto r = juce::Rectangle<int> (body.getX() + i * columns, body.getY(), columns, body.getHeight());
        g.setColour (KBColours::gold.withAlpha (0.12f));
        g.fillRect (r.getCentreX() - 2, r.getY(), 4, r.getHeight());
    }
}

void KillaBEditor::paintEqGuides (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto body = bounds.reduced (18, 26);
    const int bandW = body.getWidth() / 3;
    const char* names[] = { "LOW", "MID", "HIGH" };
    for (int i = 0; i < 3; ++i)
    {
        auto col = juce::Rectangle<int> (body.getX() + i * bandW, body.getY(), bandW, body.getHeight());
        g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 11.0f, juce::Font::bold)));
        g.setColour (KBColours::textGold);
        g.drawText (names[i], col.getX() + 10, col.getY() + 2, 40, 14, juce::Justification::left);
        g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 10.0f, juce::Font::plain)));
        g.drawText ("Q Shelf", col.getX() + 32, col.getY() + 44, 70, 14, juce::Justification::left);
        g.drawText ("FREQ", col.getX() + 10, col.getBottom() - 18, 40, 14, juce::Justification::left);
        g.setColour (KBColours::gold.withAlpha (0.16f));
        g.drawLine ((float) col.getX() + 52.0f, (float) col.getBottom() - 20.0f, (float) col.getX() + 76.0f, (float) col.getBottom() - 20.0f, 1.0f);
        g.drawLine ((float) col.getX() + 76.0f, (float) col.getBottom() - 20.0f, (float) col.getX() + 76.0f, (float) col.getY() + 96.0f, 1.0f);
    }
}

void KillaBEditor::paintFxLabels (juce::Graphics& g)
{
    for (int i = 0; i < 4; ++i)
    {
        paintPanel (g, fxCardAreas[i], FXMeta::names[i]);
    }
}

void KillaBEditor::paint (juce::Graphics& g)
{
    paintBackground (g);
    paintHeaderText (g);

    paintFxLabels (g);
    paintPanel (g, centerArea, "");
    paintPanel (g, rightArea, "");
    paintPanel (g, distArea, "DISTORTION");
    paintPanel (g, compArea, "COMPRESSOR");
    paintPanel (g, eqArea, "");
    paintPanel (g, adsrArea, "");
    paintPanel (g, mascotArea.reduced (0, 18), "");

    paintCenterBrand (g);
    paintOrb (g, orbArea.toFloat());
    paintDistortionMeters (g, distArea);
    paintEqGuides (g, eqArea);
    paintAdsrGuides (g, adsrArea);
    paintMascotPlaceholder (g, mascotArea);

    g.setColour (KBColours::gold.withAlpha (0.40f));
    g.drawVerticalLine (adsrArea.getX(), (float) bottomArea.getY() + 10.0f, (float) bottomArea.getBottom() - 10.0f);

    g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 11.0f, juce::Font::bold)).withExtraKerningFactor (0.18f));
    g.setColour (KBColours::textGold);
    g.drawText ("VOICES", centerArea.getX() + 155, centerArea.getBottom() - 78, 120, 14, juce::Justification::centred);
    g.drawText ("COMPRESSOR", compArea.getX() + 128, compArea.getY() + 104, 160, 14, juce::Justification::centred);
    g.drawText ("LFO Routing", lfoArea.getX() + 34, lfoArea.getY() + 96, 92, 14, juce::Justification::centred);
    g.drawText ("LFO Type", lfoArea.getX() + 40, lfoArea.getY() + 160, 84, 14, juce::Justification::centred);
}

void KillaBEditor::resized()
{
    auto bounds = getLocalBounds();
    headerArea = bounds.removeFromTop (86);
    keyboardArea = bounds.removeFromBottom (128);
    topArea = bounds.removeFromTop (540);
    bottomArea = bounds;

    auto top = topArea.reduced (8, 0);
    leftArea = top.removeFromLeft (356);
    rightArea = top.removeFromRight (360);
    centerArea = top.reduced (6, 0);

    {
        auto left = leftArea;
        const int gap = 16;
        auto row1 = left.removeFromTop ((left.getHeight() - gap) / 2);
        left.removeFromTop (gap);
        auto row2 = left;
        fxCardAreas[0] = row1.removeFromLeft ((row1.getWidth() - gap) / 2);
        row1.removeFromLeft (gap);
        fxCardAreas[1] = row1;
        fxCardAreas[2] = row2.removeFromLeft ((row2.getWidth() - gap) / 2);
        row2.removeFromLeft (gap);
        fxCardAreas[3] = row2;
    }

    distArea = rightArea.removeFromTop (300);
    rightArea.removeFromTop (14);
    compArea = rightArea;
    waveArea = distArea.reduced (8, 48);
    waveArea.removeFromLeft (8);
    waveArea.removeFromTop (52);
    waveDisplay.setBounds (waveArea.withTrimmedRight (36).withTrimmedBottom (108).withTrimmedLeft (6));

    auto bottom = bottomArea.reduced (20, 8);
    eqArea = bottom.removeFromLeft (560);
    adsrArea = bottom.removeFromLeft (180);
    lfoArea = bottom.removeFromLeft (180);
    mascotArea = bottom;

    presetTitleLabel.setBounds (430, 52, 150, 18);
    presetCombo.setBounds (428, 12, 300, 28);
    modeTitleLabel.setBounds (905, 52, 150, 18);
    catCombo.setBounds (868, 12, 220, 28);

    const int small = 34;
    const int large = 88;
    for (int i = 0; i < 4; ++i)
    {
        auto area = fxCardAreas[i].reduced (20, 46);
        fxAmtKnobs[i].setBounds (area.getX() + 8, area.getY() + 16, small, small);
        fxAmtLabels[i].setBounds (area.getX(), area.getY() + 48, 56, 14);
        fxDetailKnobs[i].setBounds (area.getRight() - small - 8, area.getY() + 16, small, small);
        fxDetailLabels[i].setBounds (area.getRight() - 64, area.getY() + 48, 64, 14);
        fxMixKnobs[i].setBounds (area.getCentreX() - large / 2, area.getBottom() - large - 18, large, large);
        fxMixLabels[i].setBounds (area.getCentreX() - 36, area.getBottom() - 22, 72, 14);
    }

    reverbPresetBox.setBounds (fxCardAreas[3].getX() + 44, fxCardAreas[3].getBottom() - 38, 120, 22);
    reverbPresetLabel.setBounds (0, 0, 0, 0);

    filterTypeBox.setBounds (centerArea.getX() + 52, centerArea.getY() + 10, 130, 26);
    filterTypeLabel.setBounds (centerArea.getX() + 52, centerArea.getY() + 42, 130, 14);
    filterSlopeBox.setBounds (centerArea.getX() + 212, centerArea.getY() + 10, 130, 26);
    filterSlopeLabel.setBounds (centerArea.getX() + 212, centerArea.getY() + 42, 130, 14);
    orbArea = juce::Rectangle<int> (centerArea.getX() + 70, centerArea.getY() + 164, 250, 250);
    masterGainKnob.setBounds (centerArea.getX() + 122, centerArea.getY() + 234, 146, 146);
    masterGainLabel.setBounds (centerArea.getX() + 146, centerArea.getY() + 382, 100, 14);
    glideKnob.setBounds (centerArea.getX() + 22, centerArea.getBottom() - 180, 44, 44);
    glideLabel.setBounds (centerArea.getX() + 8, centerArea.getBottom() - 142, 72, 14);
    cutoffKnob.setBounds (centerArea.getX() + 22, centerArea.getBottom() - 124, 44, 44);
    cutoffLabel.setBounds (centerArea.getX() + 0, centerArea.getBottom() - 86, 88, 14);
    voicesLabel.setBounds (centerArea.getX() + 148, centerArea.getBottom() - 70, 120, 14);
    voicesBox.setBounds (centerArea.getX() + 120, centerArea.getBottom() - 42, 170, 24);

    driveKnob.setBounds (distArea.getX() + 30, distArea.getY() + 192, 72, 72);
    driveLabel.setBounds (distArea.getX() + 20, distArea.getY() + 268, 92, 14);

    const int compStartX = compArea.getX() + 42;
    const int compY = compArea.getBottom() - 74;
    const int compStep = 62;
    compThreshKnob.setBounds (compStartX + 0 * compStep, compY, 34, 34);
    compAttackKnob.setBounds (compStartX + 1 * compStep, compY, 34, 34);
    compReleaseKnob.setBounds (compStartX + 2 * compStep, compY, 34, 34);
    compMixKnob.setBounds (compStartX + 3 * compStep, compY, 34, 34);
    compRatioKnob.setBounds (compStartX + 4 * compStep, compY, 34, 34);

    compThreshLabel.setBounds (compStartX - 14, compY + 36, 62, 14);
    compAttackLabel.setBounds (compStartX + 1 * compStep - 10, compY + 36, 56, 14);
    compReleaseLabel.setBounds (compStartX + 2 * compStep - 12, compY + 36, 64, 14);
    compMixLabel.setBounds (compStartX + 3 * compStep - 10, compY + 36, 54, 14);
    compRatioLabel.setBounds (compStartX + 4 * compStep - 10, compY + 36, 54, 14);
    masterGainKnob.toFront (false);

    const int meterY = eqArea.getY() + 36;
    const int meterH = 220;
    meterBarLow.setBounds (eqArea.getX() + 18, meterY, 16, meterH);
    meterBarMid.setBounds (eqArea.getX() + 145, meterY, 16, meterH);
    meterBarHigh.setBounds (eqArea.getX() + 272, meterY, 16, meterH);
    eqLowKnob.setBounds (eqArea.getX() + 48, eqArea.getY() + 86, 92, 92);
    eqLowLabel.setBounds (eqArea.getX() + 58, eqArea.getY() + 176, 70, 14);
    eqMidKnob.setBounds (eqArea.getX() + 175, eqArea.getY() + 86, 92, 92);
    eqMidLabel.setBounds (eqArea.getX() + 185, eqArea.getY() + 176, 70, 14);
    eqHighKnob.setBounds (eqArea.getX() + 302, eqArea.getY() + 86, 92, 92);
    eqHighLabel.setBounds (eqArea.getX() + 312, eqArea.getY() + 176, 70, 14);

    auto adsrBody = adsrArea.reduced (14, 42);
    const int sliderW = adsrBody.getWidth() / 4;
    attackSlider.setBounds (adsrBody.getX(), adsrBody.getY() + 28, sliderW, adsrBody.getHeight() - 34);
    decaySlider.setBounds (adsrBody.getX() + sliderW, adsrBody.getY() + 28, sliderW, adsrBody.getHeight() - 34);
    sustainSlider.setBounds (adsrBody.getX() + sliderW * 2, adsrBody.getY() + 28, sliderW, adsrBody.getHeight() - 34);
    releaseSlider.setBounds (adsrBody.getX() + sliderW * 3, adsrBody.getY() + 28, sliderW, adsrBody.getHeight() - 34);
    attackLabel.setBounds (adsrBody.getX(), adsrArea.getY() + 26, sliderW, 14);
    decayLabel.setBounds (adsrBody.getX() + sliderW, adsrArea.getY() + 26, sliderW, 14);
    sustainLabel.setBounds (adsrBody.getX() + sliderW * 2, adsrArea.getY() + 26, sliderW, 14);
    releaseLabel.setBounds (adsrBody.getX() + sliderW * 3, adsrArea.getY() + 26, sliderW, 14);

    lfoRoutingBox.setBounds (lfoArea.getX() + 18, lfoArea.getY() + 62, 110, 24);
    lfoRoutingLabel.setBounds (lfoArea.getX() + 18, lfoArea.getY() + 92, 110, 14);
    lfoTypeBox.setBounds (lfoArea.getX() + 18, lfoArea.getY() + 126, 110, 24);
    lfoTypeLabel.setBounds (lfoArea.getX() + 18, lfoArea.getY() + 156, 110, 14);
    lfoRateKnob.setBounds (lfoArea.getX() + 6, lfoArea.getBottom() - 98, 54, 54);
    lfoRateLabel.setBounds (lfoArea.getX() + 0, lfoArea.getBottom() - 40, 70, 14);
    lfoDepthKnob.setBounds (lfoArea.getX() + 64, lfoArea.getBottom() - 98, 54, 54);
    lfoDepthLabel.setBounds (lfoArea.getX() + 58, lfoArea.getBottom() - 40, 76, 14);

    keyboard.setBounds (keyboardArea.reduced (8, 6));

    osc1TypeBox.setBounds (0, 0, 0, 0);
    osc2TypeBox.setBounds (0, 0, 0, 0);
    osc1Label.setBounds (0, 0, 0, 0);
    osc2Label.setBounds (0, 0, 0, 0);
    oscMixKnob.setBounds (0, 0, 0, 0);
    oscMixLabel.setBounds (0, 0, 0, 0);
}
