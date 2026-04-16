#pragma once
#include <JuceHeader.h>

// =============================================================================
// KBColours — Phase 5 "Gold Edition" palette
// Deep charcoal/black base · burnished gold/bronze accents
// =============================================================================
namespace KBColours
{
    // ── Backgrounds ──────────────────────────────────────────────────────────
    const juce::Colour bg        { 0xff0b0b0f };   // Near-black
    const juce::Colour panel     { 0xff101015 };   // Dark charcoal
    const juce::Colour panel2    { 0xff15151d };   // Mid charcoal
    const juce::Colour border    { 0xff28243a };   // Dark border
    const juce::Colour divider   { 0xff201c30 };   // Panel divider

    // ── Burnished gold family ─────────────────────────────────────────────────
    const juce::Colour gold      { 0xffc8a24a };   // Mid gold
    const juce::Colour goldLight { 0xffead18e };   // Highlight gold
    const juce::Colour goldDark  { 0xff6f541d };   // Deep gold shadow
    const juce::Colour goldRim   { 0xff504028 };   // Knob rim bronze
    const juce::Colour champagne { 0xfff0dc90 };   // Champagne highlight
    const juce::Colour bronze    { 0xffb87a30 };   // Pure bronze

    // ── Distortion accent only ────────────────────────────────────────────────
    const juce::Colour orange    { 0xffb7872f };   // Amber accent (no red hue)

    // ── Text (warm gold-tinted) ───────────────────────────────────────────────
    const juce::Colour textHi    { 0xfff8f0e0 };   // Warm white
    const juce::Colour textMid   { 0xffa09060 };   // Gold-tinted grey
    const juce::Colour textDim   { 0xff605848 };   // Dim bronze
    const juce::Colour textGold  { 0xffc8a850 };   // Bright gold text

    // ── Knob materials ───────────────────────────────────────────────────────
    const juce::Colour knobBody  { 0xff181410 };   // Very dark bronze
    const juce::Colour knobRim   { 0xff504028 };   // Bronze rim

    // ── Meter — gold spectrum (amber → bright champagne) ─────────────────────
    const juce::Colour meterGreen  { 0xff907828 };   // Dim amber
    const juce::Colour meterOrange { 0xffc8963a };   // Burnished gold
    const juce::Colour meterRed    { 0xffe8c870 };   // Bright champagne

    // ── Tag resolver — everything except "orange" → gold ─────────────────────
    inline juce::Colour fromTag (const juce::String& tag)
    {
        if (tag == "orange") return orange;
        return gold;
    }
}

// =============================================================================
// KillaBLookAndFeel
// Premium gold & charcoal hardware aesthetic
// =============================================================================
class KillaBLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // ─────────────────────────────────────────────────────────────────────────
    // Constructor
    // ─────────────────────────────────────────────────────────────────────────
    KillaBLookAndFeel()
    {
        setColour (juce::Slider::rotarySliderFillColourId,     KBColours::gold);
        setColour (juce::Slider::rotarySliderOutlineColourId,  KBColours::knobBody);
        setColour (juce::Slider::thumbColourId,                KBColours::knobRim);
        setColour (juce::Slider::textBoxTextColourId,          KBColours::textMid);
        setColour (juce::Slider::textBoxBackgroundColourId,    juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId,       juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId,     KBColours::gold.withAlpha (0.4f));

        setColour (juce::Label::textColourId,       KBColours::textMid);
        setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);

        setColour (juce::ComboBox::backgroundColourId,     KBColours::panel2);
        setColour (juce::ComboBox::outlineColourId,        KBColours::goldRim);
        setColour (juce::ComboBox::textColourId,           KBColours::textHi);
        setColour (juce::ComboBox::arrowColourId,          KBColours::gold);
        setColour (juce::ComboBox::focusedOutlineColourId, KBColours::gold);

        setColour (juce::PopupMenu::backgroundColourId,             KBColours::panel);
        setColour (juce::PopupMenu::textColourId,                   KBColours::textHi);
        setColour (juce::PopupMenu::highlightedBackgroundColourId,  KBColours::gold.withAlpha (0.5f));
        setColour (juce::PopupMenu::highlightedTextColourId,        KBColours::textHi);

        setColour (juce::TextEditor::backgroundColourId,     juce::Colours::transparentBlack);
        setColour (juce::TextEditor::textColourId,           KBColours::textMid);
        setColour (juce::TextEditor::outlineColourId,        juce::Colours::transparentBlack);
        setColour (juce::TextEditor::focusedOutlineColourId, KBColours::gold.withAlpha (0.5f));
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Plugin background — brushed metal charcoal + gold header line
    // ─────────────────────────────────────────────────────────────────────────
    static void drawPluginBackground (juce::Graphics& g, int w, int h)
    {
        // 1. Base fill — near-black
        g.setColour (KBColours::bg);
        g.fillRect (0, 0, w, h);

        // 2. Diagonal brushed-metal grain
        g.setColour (juce::Colours::white.withAlpha (0.010f));
        for (int d = 0; d < w + h; d += 4)
        {
            const float x1 = (float) juce::jmax (0, d - h);
            const float y1 = (float) juce::jmin (h, d);
            const float x2 = (float) juce::jmin (w, d);
            const float y2 = (float) juce::jmax (0, d - w);
            g.drawLine (x1, y1, x2, y2, 0.4f);
        }

        // 3. Subtle crosshatch grid
        g.setColour (juce::Colours::white.withAlpha (0.013f));
        for (int x = 0; x < w; x += 20) g.drawVerticalLine   (x, 0.0f, (float) h);
        for (int y = 0; y < h; y += 20) g.drawHorizontalLine (y, 0.0f, (float) w);

        // 4. Centre radial highlight (very subtle warmth)
        {
            const float r = (float) juce::jmax (w, h) * 0.65f;
            juce::ColourGradient hl (juce::Colours::white.withAlpha (0.022f),
                                     (float) w * 0.5f, (float) h * 0.34f,
                                     juce::Colours::transparentBlack,
                                     (float) w * 0.5f + r, (float) h * 0.34f, true);
            g.setGradientFill (hl);
            g.fillRect (0, 0, w, h);
        }

        // 5. Edge vignette — darker corners
        {
            const float r = (float) juce::jmax (w, h) * 0.75f;
            juce::ColourGradient vig (juce::Colours::transparentBlack,
                                      (float) w * 0.5f, (float) h * 0.5f,
                                      juce::Colours::black.withAlpha (0.62f),
                                      (float) w * 0.5f + r, (float) h * 0.5f, true);
            g.setGradientFill (vig);
            g.fillRect (0, 0, w, h);
        }

        // 5b. Tight edge vignette ring for cinematic focus.
        {
            const float r = (float) juce::jmax (w, h) * 0.58f;
            juce::ColourGradient vig2 (juce::Colours::transparentBlack,
                                       (float) w * 0.5f, (float) h * 0.48f,
                                       juce::Colours::black.withAlpha (0.32f),
                                       (float) w * 0.5f + r, (float) h * 0.48f, true);
            g.setGradientFill (vig2);
            g.fillRect (0, 0, w, h);
        }

        // 5c. Very light noise grain overlay (premium matte surface).
        {
            g.setColour (juce::Colours::white.withAlpha (0.055f));
            for (int y = 0; y < h; y += 3)
            {
                const int x = (y * 37) % juce::jmax (1, w);
                g.fillRect (x, y, 1, 1);
            }
            g.setColour (juce::Colours::black.withAlpha (0.05f));
            for (int x = 0; x < w; x += 4)
            {
                const int y = (x * 29 + 11) % juce::jmax (1, h);
                g.fillRect (x, y, 1, 1);
            }
        }

        // 6. Header bar — subtle gradient charcoal
        {
            juce::ColourGradient hdr (KBColours::panel.brighter (0.12f), 0.0f,  0.0f,
                                       KBColours::panel,                  0.0f, 60.0f, false);
            g.setGradientFill (hdr);
            g.fillRect (0, 0, w, 60);
        }

        // 7. Header bottom — sweeping gold gradient line
        {
            juce::ColourGradient gLine (juce::Colours::transparentBlack,
                                         0.0f, 61.0f,
                                         juce::Colours::transparentBlack,
                                         (float) w, 61.0f, false);
            gLine.addColour (0.05, KBColours::gold.withAlpha (0.75f));
            gLine.addColour (0.50, KBColours::goldLight);
            gLine.addColour (0.95, KBColours::gold.withAlpha (0.75f));
            g.setGradientFill (gLine);
            g.fillRect (0, 60, w, 2);
        }

        // 8. Bottom gold glow strip
        {
            juce::ColourGradient glow (KBColours::gold.withAlpha (0.12f),
                                        (float) w * 0.5f, (float) h,
                                        juce::Colours::transparentBlack,
                                        (float) w * 0.5f, (float)(h - 34), false);
            g.setGradientFill (glow);
            g.fillRect (0, h - 34, w, 34);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Burnished gold / bronze 3D rotary knob
    // ─────────────────────────────────────────────────────────────────────────
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        const juce::String colTag     = slider.getProperties().getWithDefault ("kbColour", "gold");
        const juce::Colour accentCol  = KBColours::fromTag (colTag);

        const float cx     = (float) x + (float) width  * 0.5f;
        const float cy     = (float) y + (float) height * 0.5f;
        const float baseRadius = (float) juce::jmin (width, height) * 0.5f - 4.0f;
        const float angle  = rotaryStartAngle
                             + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        const bool isHover = slider.isMouseOverOrDragging();
        const bool isActive = isHover || sliderPosProportional > 0.01f;
        const bool glowEnabled = slider.getProperties().getWithDefault ("kbGlowEnabled", false);
        const float hoverScale = isHover ? 1.04f : 1.0f;
        const float radius = baseRadius * hoverScale;
        constexpr float kGlowBase = 0.12f;
        constexpr float kGlowHover = 0.19f;

        // ── 0. Active outer glow ────────────────────────────────────────────
        if (glowEnabled && isActive)
        {
            for (int i = 4; i >= 1; --i)
            {
                const float s = radius + (float) i * 3.2f;
                g.setColour (accentCol.withAlpha ((isHover ? kGlowHover : kGlowBase) / (float) i));
                g.fillEllipse (cx - s, cy - s, s * 2.0f, s * 2.0f);
            }
        }

        // ── 1. Drop shadow ──────────────────────────────────────────────────
        for (int i = 5; i >= 1; --i)
        {
            const float s = radius + (float) i * 1.6f;
            g.setColour (juce::Colours::black.withAlpha (0.28f / (float) i));
            g.fillEllipse (cx - s, cy - s + 4.0f, s * 2.0f, s * 2.0f);
        }

        // ── 2. Arc track groove ─────────────────────────────────────────────
        {
            juce::Path track;
            track.addCentredArc (cx, cy, radius - 1.5f, radius - 1.5f,
                                 0.0f, rotaryStartAngle, rotaryEndAngle, true);
            g.setColour (KBColours::knobBody.brighter (0.08f));
            g.strokePath (track, juce::PathStrokeType (3.5f,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // ── 3. Value arc — glow layers then solid ───────────────────────────
        if (sliderPosProportional > 0.002f)
        {
            juce::Path val;
            val.addCentredArc (cx, cy, radius - 1.5f, radius - 1.5f,
                               0.0f, rotaryStartAngle, angle, true);
            g.setColour (accentCol.withAlpha (0.20f));
            g.strokePath (val, juce::PathStrokeType (11.0f,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour (accentCol.withAlpha (0.40f));
            g.strokePath (val, juce::PathStrokeType (6.0f,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour (accentCol.brighter (0.15f));
            g.strokePath (val, juce::PathStrokeType (2.5f,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // ── 4. Outer rim — dark bronze radial gradient ──────────────────────
        {
            juce::ColourGradient rimGrad (
                KBColours::goldRim.brighter (0.55f),
                cx - radius * 0.55f, cy - radius * 0.55f,
                KBColours::goldRim.darker  (0.70f),
                cx + radius * 0.55f, cy + radius * 0.55f, true);
            g.setGradientFill (rimGrad);
            g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        }

        // ── 5. Tick marks — 11 evenly spaced around arc ─────────────────────
        {
            constexpr int N = 11;
            const float tickOuter = radius - 0.5f;
            const float tickInner = radius - radius * 0.16f;
            for (int ti = 0; ti < N; ++ti)
            {
                const float ta = rotaryStartAngle
                                 + (float) ti / (float)(N - 1)
                                   * (rotaryEndAngle - rotaryStartAngle);
                // JUCE rotary convention: 0 = top, clockwise positive
                // x = cx + sin(ta) * r,  y = cy - cos(ta) * r
                const float sA = std::sin (ta);
                const float cA = std::cos (ta);

                const bool major = (ti == 0 || ti == N - 1 || ti == N / 2);
                const float inner = major ? (radius - radius * 0.22f) : tickInner;

                g.setColour (major ? KBColours::gold.withAlpha (0.75f)
                                   : KBColours::goldRim.brighter (0.35f).withAlpha (0.45f));
                g.drawLine (cx + sA * tickOuter, cy - cA * tickOuter,
                            cx + sA * inner,     cy - cA * inner,
                            major ? 1.4f : 0.7f);
            }
        }

        // ── 6. Knob face — burnished gold radial gradient ───────────────────
        {
            const float bR = radius - 4.5f;
            juce::ColourGradient face (
                KBColours::goldLight.withAlpha (0.95f),
                cx - bR * 0.38f, cy - bR * 0.48f,
                KBColours::goldDark,
                cx + bR * 0.55f, cy + bR * 0.55f, true);
            face.addColour (0.45, KBColours::gold);
            g.setGradientFill (face);
            g.fillEllipse (cx - bR, cy - bR, bR * 2.0f, bR * 2.0f);
        }

        // ── 7. Inner groove ring — thin dark line ───────────────────────────
        {
            const float gR = radius - 4.0f;
            g.setColour (KBColours::goldDark.darker (0.7f));
            g.drawEllipse (cx - gR, cy - gR, gR * 2.0f, gR * 2.0f, 1.2f);
        }

        // ── 8. Centre engraving — small depressed circle ────────────────────
        {
            const float dR = radius * 0.26f;
            juce::ColourGradient det (
                KBColours::goldDark.darker (0.5f), cx - dR, cy - dR,
                KBColours::gold.withAlpha (0.25f), cx + dR, cy + dR, true);
            g.setGradientFill (det);
            g.fillEllipse (cx - dR, cy - dR, dR * 2.0f, dR * 2.0f);
            g.setColour (KBColours::goldRim.darker (0.4f));
            g.drawEllipse (cx - dR, cy - dR, dR * 2.0f, dR * 2.0f, 0.8f);
        }

        // ── 9. Indicator line — tapered, from center to near rim ────────────
        {
            const float lineStart = radius * 0.24f;
            const float lineEnd   = radius - 5.5f;
            // JUCE rotary convention: x = cx + sin(angle)*r, y = cy - cos(angle)*r
            const float sA = std::sin (angle);
            const float cA = std::cos (angle);

            const float ix1 = cx + sA * lineStart;
            const float iy1 = cy - cA * lineStart;
            const float ix2 = cx + sA * lineEnd;
            const float iy2 = cy - cA * lineEnd;

            // Wide glow
            g.setColour (accentCol.withAlpha (0.40f));
            g.drawLine (ix1, iy1, ix2, iy2, 4.5f);
            // Bright core
            g.setColour (KBColours::champagne);
            g.drawLine (ix1, iy1, ix2, iy2, 1.6f);
            // Tip dot
            g.setColour (KBColours::champagne.brighter (0.4f));
            g.fillEllipse (ix2 - 2.8f, iy2 - 2.8f, 5.6f, 5.6f);
        }

        // ── 10. Glass highlight — upper-left soft ellipse ────────────────────
        {
            const float bR = radius - 4.5f;
            const float hlW = bR * 0.50f;
            const float hlH = bR * 0.30f;
            const float hlX = cx - bR * 0.34f;
            const float hlY = cy - bR * 0.60f;
            juce::ColourGradient hl (
                juce::Colours::white.withAlpha (0.28f), hlX,       hlY,
                juce::Colours::white.withAlpha (0.0f),  hlX + hlW, hlY + hlH, false);
            g.setGradientFill (hl);
            g.fillEllipse (hlX, hlY, hlW, hlH);
        }

        // ── 11. Inner shadow vignette for depth ──────────────────────────────
        {
            const float bR = radius - 4.5f;
            juce::ColourGradient innerShadow (juce::Colours::transparentBlack, cx, cy,
                                              juce::Colours::black.withAlpha (0.22f),
                                              cx + bR, cy + bR, true);
            g.setGradientFill (innerShadow);
            g.fillEllipse (cx - bR, cy - bR, bR * 2.0f, bR * 2.0f);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Thin glass-rod vertical slider — metal end-caps, gold fill
    // ─────────────────────────────────────────────────────────────────────────
    void drawLinearSlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float /*minSliderPos*/, float /*maxSliderPos*/,
                           juce::Slider::SliderStyle style,
                           juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearVertical)
        {
            LookAndFeel_V4::drawLinearSlider (g, x, y, width, height,
                                              sliderPos, 0.0f, (float) height,
                                              style, slider);
            return;
        }

        const juce::String colTag    = slider.getProperties().getWithDefault ("kbColour", "gold");
        const juce::Colour accentCol = KBColours::fromTag (colTag);
        const bool isActive = slider.isMouseOverOrDragging();
        const bool isAdsrSlider = slider.getProperties().getWithDefault ("kbAdsrSlider", false);

        const float cx       = (float) x + (float) width * 0.5f;
        const float rodW     = isAdsrSlider ? 5.0f : 7.0f;
        const float rodX     = cx - rodW * 0.5f;
        const float capH     = isAdsrSlider ? 0.0f : 10.0f;
        const float trackTop = (float) y + capH;
        const float trackBot = (float)(y + height) - capH;

        // ── Top metal cap ─────────────────────────────────────────────────────
        if (! isAdsrSlider)
        {
            juce::ColourGradient cap (
                KBColours::goldLight.withAlpha (0.9f), rodX,          (float) y,
                KBColours::goldDark,                   rodX + rodW,  (float) y + capH, false);
            g.setGradientFill (cap);
            g.fillRoundedRectangle (rodX - 2.5f, (float) y, rodW + 5.0f, capH, 3.0f);
            g.setColour (KBColours::champagne.withAlpha (0.4f));
            g.drawRoundedRectangle (rodX - 2.5f, (float) y, rodW + 5.0f, capH, 3.0f, 0.7f);
        }

        // ── Rod body — dark tube ──────────────────────────────────────────────
        g.setColour (isAdsrSlider ? KBColours::gold.withAlpha (0.10f)
                                  : KBColours::knobBody.brighter (0.06f));
        g.fillRoundedRectangle (rodX, trackTop, rodW, trackBot - trackTop, 2.5f);
        g.setColour (isAdsrSlider ? juce::Colours::transparentBlack
                                  : KBColours::goldRim.withAlpha (0.28f));
        g.drawRoundedRectangle (rodX, trackTop, rodW, trackBot - trackTop, 2.5f, 0.7f);

        // Subtle specular line on rod
        g.setColour (isAdsrSlider ? juce::Colours::transparentBlack
                                  : juce::Colours::white.withAlpha (0.07f));
        g.fillRoundedRectangle (rodX + 1.2f, trackTop, 2.2f, trackBot - trackTop, 1.5f);

        // ── Fill from bottom to thumb ─────────────────────────────────────────
        if (sliderPos < trackBot - 2.0f)
        {
            const float fillH = trackBot - sliderPos;
            // Soft glow halo
            g.setColour (accentCol.withAlpha (0.16f));
            g.fillRoundedRectangle (rodX - 3.0f, sliderPos, rodW + 6.0f, fillH, 3.0f);
            // Fill body
            juce::ColourGradient fill (
                accentCol.brighter (0.25f), cx, trackBot,
                accentCol.withAlpha (0.55f), cx, sliderPos, false);
            g.setGradientFill (fill);
            g.fillRoundedRectangle (rodX + 1.0f, sliderPos + 1.0f,
                                    rodW - 2.0f, fillH - 2.0f, 2.0f);
        }

        // ── Metallic thumb bar ────────────────────────────────────────────────
        {
            const float tH = 13.0f;
            const float tW = rodW + 16.0f;
            const float tX = cx - tW * 0.5f;
            const float tY = sliderPos - tH * 0.5f;

            if (isActive)
            {
                g.setColour (accentCol.withAlpha (0.24f));
                g.fillRoundedRectangle (tX - 4.0f, tY - 4.0f, tW + 8.0f, tH + 8.0f, 4.0f);
            }

            // Shadow
            g.setColour (juce::Colours::black.withAlpha (0.45f));
            g.fillRoundedRectangle (tX + 1.0f, tY + 2.5f, tW, tH, 3.0f);

            // Thumb body — brushed gold gradient
            juce::ColourGradient thumb (
                KBColours::goldLight, tX + tW * 0.2f, tY,
                KBColours::goldDark,  tX + tW * 0.8f, tY + tH, false);
            g.setGradientFill (thumb);
            g.fillRoundedRectangle (tX, tY, tW, tH, 3.0f);

            // Border
            g.setColour (KBColours::champagne.withAlpha (0.55f));
            g.drawRoundedRectangle (tX, tY, tW, tH, 3.0f, 0.8f);

            // Top highlight strip
            g.setColour (juce::Colours::white.withAlpha (0.30f));
            g.fillRoundedRectangle (tX + 2.5f, tY + 2.0f, tW - 5.0f, 3.0f, 1.5f);

            // Centre groove line
            g.setColour (KBColours::goldDark.darker (0.3f));
            g.drawHorizontalLine ((int)(tY + tH * 0.5f),
                                  tX + 4.0f, tX + tW - 4.0f);
        }

        // ── Bottom metal cap ──────────────────────────────────────────────────
        if (! isAdsrSlider)
        {
            juce::ColourGradient cap (
                KBColours::goldDark,                  rodX,         trackBot - capH,
                KBColours::goldLight.withAlpha (0.6f), rodX + rodW, trackBot, false);
            g.setGradientFill (cap);
            g.fillRoundedRectangle (rodX - 2.5f, trackBot - capH,
                                    rodW + 5.0f, capH, 3.0f);
            g.setColour (KBColours::champagne.withAlpha (0.3f));
            g.drawRoundedRectangle (rodX - 2.5f, trackBot - capH,
                                    rodW + 5.0f, capH, 3.0f, 0.7f);
        }
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KillaBLookAndFeel)
};
