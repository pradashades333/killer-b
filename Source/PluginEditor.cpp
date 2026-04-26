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

namespace
{
    constexpr int designWidth = 1110;
    constexpr int designHeight = 1110;
    constexpr int screenFitMargin = 72;
    constexpr float minimumDisplayFitScale = 0.45f;

    struct PresetParamValue
    {
        const char* id;
        float value;
    };

    struct ComboPresetState
    {
        int modeId;
        int filterTypeId;
        int filterSlopeId;
        int voicesId;
        int reverbPresetId;
        int lfoRoutingId;
        int lfoTypeId;
    };

    struct UiKnobPresetState
    {
        float fxDetail[4];
        float cutoff;
        float compMix;
        float lfoRate;
        float lfoDepth;
    };

    struct FactoryPreset
    {
        const char* name;
        PresetParamValue params[26];
        ComboPresetState combos;
        UiKnobPresetState uiKnobs;
        int drumKitId = -1;
    };

    constexpr PresetParamValue makeParam (const char* id, float value) noexcept
    {
        return { id, value };
    }

    static const std::array<FactoryPreset, 37> factoryPresets
    {{
        { "Bass - Dark Moog",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 1.0f), makeParam ("oscmix", 0.70f),
            makeParam ("attack", 0.004f), makeParam ("decay", 0.12f), makeParam ("sustain", 0.62f), makeParam ("release", 0.14f),
            makeParam ("glide", 0.08f), makeParam ("drive", 8.5f), makeParam ("filterCutoff", 280.0f),
            makeParam ("CHORUS_AMOUNT", 0.05f), makeParam ("CHORUS_MIX", 0.02f),
            makeParam ("DELAY_AMOUNT", 0.18f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.10f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.12f), makeParam ("REVERB_MIX", 0.02f),
            makeParam ("LOW_GAIN", 9.0f), makeParam ("MID_GAIN", -6.0f), makeParam ("HIGH_GAIN", -12.0f),
            makeParam ("MASTER_GAIN", -2.0f),
            makeParam ("COMP_THRESHOLD", -18.0f), makeParam ("COMP_RATIO", 4.5f), makeParam ("COMP_ATTACK", 5.0f), makeParam ("COMP_RELEASE", 90.0f) },
          { 2, 2, 2, 3, 2, 1, 1 },
          { { 0.15f, 0.05f, 0.10f, 0.18f }, 0.22f, 0.28f, 0.08f, 0.04f } },

        { "Bass - Sub Pressure",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.15f),
            makeParam ("attack", 0.006f), makeParam ("decay", 0.45f), makeParam ("sustain", 0.90f), makeParam ("release", 0.55f),
            makeParam ("glide", 0.01f), makeParam ("drive", 1.2f), makeParam ("filterCutoff", 140.0f),
            makeParam ("CHORUS_AMOUNT", 0.02f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.20f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.06f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.08f), makeParam ("REVERB_MIX", 0.00f),
            makeParam ("LOW_GAIN", 12.0f), makeParam ("MID_GAIN", -10.0f), makeParam ("HIGH_GAIN", -18.0f),
            makeParam ("MASTER_GAIN", 3.0f),
            makeParam ("COMP_THRESHOLD", -14.0f), makeParam ("COMP_RATIO", 3.5f), makeParam ("COMP_ATTACK", 18.0f), makeParam ("COMP_RELEASE", 220.0f) },
          { 2, 2, 2, 2, 1, 1, 1 },
          { { 0.08f, 0.00f, 0.00f, 0.12f }, 0.16f, 0.35f, 0.05f, 0.02f } },

        { "Bass - Rubber Growl",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.55f),
            makeParam ("attack", 0.008f), makeParam ("decay", 0.18f), makeParam ("sustain", 0.48f), makeParam ("release", 0.18f),
            makeParam ("glide", 0.16f), makeParam ("drive", 12.0f), makeParam ("filterCutoff", 650.0f),
            makeParam ("CHORUS_AMOUNT", 0.12f), makeParam ("CHORUS_MIX", 0.06f),
            makeParam ("DELAY_AMOUNT", 0.12f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.55f), makeParam ("PHASER_MIX", 0.25f),
            makeParam ("REVERB_AMOUNT", 0.14f), makeParam ("REVERB_MIX", 0.02f),
            makeParam ("LOW_GAIN", 5.0f), makeParam ("MID_GAIN", 7.0f), makeParam ("HIGH_GAIN", -8.0f),
            makeParam ("MASTER_GAIN", -3.0f),
            makeParam ("COMP_THRESHOLD", -20.0f), makeParam ("COMP_RATIO", 5.0f), makeParam ("COMP_ATTACK", 4.0f), makeParam ("COMP_RELEASE", 80.0f) },
          { 3, 2, 2, 3, 1, 3, 2 },
          { { 0.18f, 0.00f, 0.20f, 0.08f }, 0.28f, 0.40f, 0.24f, 0.22f } },

        { "Lead - Hard Neon",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.52f),
            makeParam ("attack", 0.002f), makeParam ("decay", 0.10f), makeParam ("sustain", 0.88f), makeParam ("release", 0.12f),
            makeParam ("glide", 0.22f), makeParam ("drive", 6.2f), makeParam ("filterCutoff", 7200.0f),
            makeParam ("CHORUS_AMOUNT", 0.18f), makeParam ("CHORUS_MIX", 0.07f),
            makeParam ("DELAY_AMOUNT", 0.33f), makeParam ("DELAY_MIX", 0.26f),
            makeParam ("PHASER_AMOUNT", 0.14f), makeParam ("PHASER_MIX", 0.04f),
            makeParam ("REVERB_AMOUNT", 0.24f), makeParam ("REVERB_MIX", 0.09f),
            makeParam ("LOW_GAIN", -7.0f), makeParam ("MID_GAIN", 6.5f), makeParam ("HIGH_GAIN", 8.0f),
            makeParam ("MASTER_GAIN", -2.0f),
            makeParam ("COMP_THRESHOLD", -14.0f), makeParam ("COMP_RATIO", 2.8f), makeParam ("COMP_ATTACK", 3.0f), makeParam ("COMP_RELEASE", 80.0f) },
          { 3, 2, 1, 2, 2, 2, 3 },
          { { 0.22f, 0.42f, 0.10f, 0.26f }, 0.72f, 0.18f, 0.36f, 0.20f } },

        { "Lead - Silk Solo",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.32f),
            makeParam ("attack", 0.018f), makeParam ("decay", 0.36f), makeParam ("sustain", 0.68f), makeParam ("release", 0.62f),
            makeParam ("glide", 0.18f), makeParam ("drive", 1.6f), makeParam ("filterCutoff", 5200.0f),
            makeParam ("CHORUS_AMOUNT", 0.34f), makeParam ("CHORUS_MIX", 0.24f),
            makeParam ("DELAY_AMOUNT", 0.42f), makeParam ("DELAY_MIX", 0.22f),
            makeParam ("PHASER_AMOUNT", 0.12f), makeParam ("PHASER_MIX", 0.02f),
            makeParam ("REVERB_AMOUNT", 0.48f), makeParam ("REVERB_MIX", 0.24f),
            makeParam ("LOW_GAIN", -4.0f), makeParam ("MID_GAIN", 2.0f), makeParam ("HIGH_GAIN", 5.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -12.0f), makeParam ("COMP_RATIO", 2.2f), makeParam ("COMP_ATTACK", 28.0f), makeParam ("COMP_RELEASE", 180.0f) },
          { 2, 2, 1, 2, 2, 2, 1 },
          { { 0.18f, 0.30f, 0.06f, 0.34f }, 0.56f, 0.14f, 0.28f, 0.16f } },

        { "Lead - Razor Glide",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.70f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.08f), makeParam ("sustain", 0.72f), makeParam ("release", 0.10f),
            makeParam ("glide", 0.42f), makeParam ("drive", 10.0f), makeParam ("filterCutoff", 260.0f),
            makeParam ("CHORUS_AMOUNT", 0.06f), makeParam ("CHORUS_MIX", 0.02f),
            makeParam ("DELAY_AMOUNT", 0.24f), makeParam ("DELAY_MIX", 0.14f),
            makeParam ("PHASER_AMOUNT", 0.40f), makeParam ("PHASER_MIX", 0.16f),
            makeParam ("REVERB_AMOUNT", 0.14f), makeParam ("REVERB_MIX", 0.04f),
            makeParam ("LOW_GAIN", -10.0f), makeParam ("MID_GAIN", 8.0f), makeParam ("HIGH_GAIN", 9.0f),
            makeParam ("MASTER_GAIN", -3.0f),
            makeParam ("COMP_THRESHOLD", -18.0f), makeParam ("COMP_RATIO", 4.5f), makeParam ("COMP_ATTACK", 2.0f), makeParam ("COMP_RELEASE", 70.0f) },
          { 3, 3, 1, 3, 1, 2, 3 },
          { { 0.06f, 0.20f, 0.18f, 0.10f }, 0.80f, 0.22f, 0.42f, 0.26f } },

        { "Pluck - Velvet Bell",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.36f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.18f), makeParam ("sustain", 0.02f), makeParam ("release", 0.22f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.1f), makeParam ("filterCutoff", 9500.0f),
            makeParam ("CHORUS_AMOUNT", 0.26f), makeParam ("CHORUS_MIX", 0.16f),
            makeParam ("DELAY_AMOUNT", 0.50f), makeParam ("DELAY_MIX", 0.28f),
            makeParam ("PHASER_AMOUNT", 0.07f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.56f), makeParam ("REVERB_MIX", 0.30f),
            makeParam ("LOW_GAIN", -8.0f), makeParam ("MID_GAIN", -1.0f), makeParam ("HIGH_GAIN", 8.0f),
            makeParam ("MASTER_GAIN", 2.0f),
            makeParam ("COMP_THRESHOLD", -16.0f), makeParam ("COMP_RATIO", 2.4f), makeParam ("COMP_ATTACK", 18.0f), makeParam ("COMP_RELEASE", 180.0f) },
          { 1, 2, 1, 1, 3, 1, 1 },
          { { 0.14f, 0.26f, 0.04f, 0.38f }, 0.48f, 0.12f, 0.10f, 0.06f } },

        { "Pluck - Glass Wire",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.58f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.10f), makeParam ("sustain", 0.00f), makeParam ("release", 0.08f),
            makeParam ("glide", 0.00f), makeParam ("drive", 2.4f), makeParam ("filterCutoff", 1200.0f),
            makeParam ("CHORUS_AMOUNT", 0.12f), makeParam ("CHORUS_MIX", 0.04f),
            makeParam ("DELAY_AMOUNT", 0.18f), makeParam ("DELAY_MIX", 0.08f),
            makeParam ("PHASER_AMOUNT", 0.62f), makeParam ("PHASER_MIX", 0.20f),
            makeParam ("REVERB_AMOUNT", 0.22f), makeParam ("REVERB_MIX", 0.08f),
            makeParam ("LOW_GAIN", -14.0f), makeParam ("MID_GAIN", 3.0f), makeParam ("HIGH_GAIN", 11.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -18.0f), makeParam ("COMP_RATIO", 4.0f), makeParam ("COMP_ATTACK", 2.0f), makeParam ("COMP_RELEASE", 60.0f) },
          { 1, 3, 2, 1, 3, 2, 2 },
          { { 0.08f, 0.18f, 0.18f, 0.30f }, 0.74f, 0.16f, 0.20f, 0.14f } },

        { "Pluck - Resin Pop",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 1.0f), makeParam ("oscmix", 0.48f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.07f), makeParam ("sustain", 0.00f), makeParam ("release", 0.06f),
            makeParam ("glide", 0.01f), makeParam ("drive", 6.4f), makeParam ("filterCutoff", 3400.0f),
            makeParam ("CHORUS_AMOUNT", 0.10f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.13f), makeParam ("DELAY_MIX", 0.04f),
            makeParam ("PHASER_AMOUNT", 0.11f), makeParam ("PHASER_MIX", 0.03f),
            makeParam ("REVERB_AMOUNT", 0.16f), makeParam ("REVERB_MIX", 0.04f),
            makeParam ("LOW_GAIN", -6.0f), makeParam ("MID_GAIN", 7.0f), makeParam ("HIGH_GAIN", 2.0f),
            makeParam ("MASTER_GAIN", -2.0f),
            makeParam ("COMP_THRESHOLD", -20.0f), makeParam ("COMP_RATIO", 5.0f), makeParam ("COMP_ATTACK", 1.0f), makeParam ("COMP_RELEASE", 45.0f) },
          { 1, 2, 2, 2, 2, 3, 3 },
          { { 0.10f, 0.10f, 0.10f, 0.18f }, 0.34f, 0.18f, 0.26f, 0.20f } },

        { "Synth - Chrome Motion",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 1.0f), makeParam ("oscmix", 0.56f),
            makeParam ("attack", 0.030f), makeParam ("decay", 0.55f), makeParam ("sustain", 0.58f), makeParam ("release", 0.70f),
            makeParam ("glide", 0.08f), makeParam ("drive", 2.0f), makeParam ("filterCutoff", 4800.0f),
            makeParam ("CHORUS_AMOUNT", 0.42f), makeParam ("CHORUS_MIX", 0.28f),
            makeParam ("DELAY_AMOUNT", 0.56f), makeParam ("DELAY_MIX", 0.22f),
            makeParam ("PHASER_AMOUNT", 0.72f), makeParam ("PHASER_MIX", 0.32f),
            makeParam ("REVERB_AMOUNT", 0.42f), makeParam ("REVERB_MIX", 0.18f),
            makeParam ("LOW_GAIN", -2.0f), makeParam ("MID_GAIN", 4.0f), makeParam ("HIGH_GAIN", 4.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -16.0f), makeParam ("COMP_RATIO", 3.2f), makeParam ("COMP_ATTACK", 18.0f), makeParam ("COMP_RELEASE", 220.0f) },
          { 1, 2, 1, 1, 2, 3, 2 },
          { { 0.20f, 0.32f, 0.22f, 0.34f }, 0.68f, 0.22f, 0.32f, 0.24f } },

        { "Synth - Arcade Drift",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 1.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 0.004f), makeParam ("decay", 0.16f), makeParam ("sustain", 0.42f), makeParam ("release", 0.24f),
            makeParam ("glide", 0.04f), makeParam ("drive", 4.8f), makeParam ("filterCutoff", 900.0f),
            makeParam ("CHORUS_AMOUNT", 0.10f), makeParam ("CHORUS_MIX", 0.05f),
            makeParam ("DELAY_AMOUNT", 0.25f), makeParam ("DELAY_MIX", 0.12f),
            makeParam ("PHASER_AMOUNT", 0.34f), makeParam ("PHASER_MIX", 0.18f),
            makeParam ("REVERB_AMOUNT", 0.18f), makeParam ("REVERB_MIX", 0.06f),
            makeParam ("LOW_GAIN", -8.0f), makeParam ("MID_GAIN", 6.0f), makeParam ("HIGH_GAIN", 6.5f),
            makeParam ("MASTER_GAIN", -1.0f),
            makeParam ("COMP_THRESHOLD", -18.0f), makeParam ("COMP_RATIO", 4.0f), makeParam ("COMP_ATTACK", 5.0f), makeParam ("COMP_RELEASE", 95.0f) },
          { 2, 3, 1, 2, 1, 2, 3 },
          { { 0.16f, 0.24f, 0.28f, 0.20f }, 0.62f, 0.20f, 0.40f, 0.28f } },

        { "Synth - Analog Runner",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.68f),
            makeParam ("attack", 0.010f), makeParam ("decay", 0.32f), makeParam ("sustain", 0.70f), makeParam ("release", 0.50f),
            makeParam ("glide", 0.24f), makeParam ("drive", 5.6f), makeParam ("filterCutoff", 1800.0f),
            makeParam ("CHORUS_AMOUNT", 0.18f), makeParam ("CHORUS_MIX", 0.10f),
            makeParam ("DELAY_AMOUNT", 0.38f), makeParam ("DELAY_MIX", 0.16f),
            makeParam ("PHASER_AMOUNT", 0.08f), makeParam ("PHASER_MIX", 0.02f),
            makeParam ("REVERB_AMOUNT", 0.20f), makeParam ("REVERB_MIX", 0.06f),
            makeParam ("LOW_GAIN", 5.0f), makeParam ("MID_GAIN", 1.0f), makeParam ("HIGH_GAIN", -4.0f),
            makeParam ("MASTER_GAIN", -2.0f),
            makeParam ("COMP_THRESHOLD", -18.0f), makeParam ("COMP_RATIO", 3.5f), makeParam ("COMP_ATTACK", 7.0f), makeParam ("COMP_RELEASE", 115.0f) },
          { 3, 2, 1, 3, 1, 2, 1 },
          { { 0.10f, 0.15f, 0.06f, 0.12f }, 0.44f, 0.24f, 0.22f, 0.18f } },

        { "Pad - Honey Cloud",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.28f),
            makeParam ("attack", 1.20f), makeParam ("decay", 1.60f), makeParam ("sustain", 0.88f), makeParam ("release", 3.20f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.0f), makeParam ("filterCutoff", 2600.0f),
            makeParam ("CHORUS_AMOUNT", 0.55f), makeParam ("CHORUS_MIX", 0.38f),
            makeParam ("DELAY_AMOUNT", 0.62f), makeParam ("DELAY_MIX", 0.18f),
            makeParam ("PHASER_AMOUNT", 0.10f), makeParam ("PHASER_MIX", 0.03f),
            makeParam ("REVERB_AMOUNT", 0.82f), makeParam ("REVERB_MIX", 0.44f),
            makeParam ("LOW_GAIN", 2.0f), makeParam ("MID_GAIN", -4.0f), makeParam ("HIGH_GAIN", 5.0f),
            makeParam ("MASTER_GAIN", -1.0f),
            makeParam ("COMP_THRESHOLD", -8.0f), makeParam ("COMP_RATIO", 1.5f), makeParam ("COMP_ATTACK", 60.0f), makeParam ("COMP_RELEASE", 420.0f) },
          { 1, 2, 1, 1, 2, 3, 1 },
          { { 0.24f, 0.20f, 0.10f, 0.52f }, 0.40f, 0.10f, 0.18f, 0.20f } },

        { "Pad - Cinema Bloom",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 1.80f), makeParam ("decay", 2.00f), makeParam ("sustain", 0.76f), makeParam ("release", 4.20f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.1f), makeParam ("filterCutoff", 6800.0f),
            makeParam ("CHORUS_AMOUNT", 0.36f), makeParam ("CHORUS_MIX", 0.24f),
            makeParam ("DELAY_AMOUNT", 0.74f), makeParam ("DELAY_MIX", 0.22f),
            makeParam ("PHASER_AMOUNT", 0.46f), makeParam ("PHASER_MIX", 0.18f),
            makeParam ("REVERB_AMOUNT", 0.92f), makeParam ("REVERB_MIX", 0.50f),
            makeParam ("LOW_GAIN", -3.0f), makeParam ("MID_GAIN", -2.0f), makeParam ("HIGH_GAIN", 7.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -10.0f), makeParam ("COMP_RATIO", 1.6f), makeParam ("COMP_ATTACK", 80.0f), makeParam ("COMP_RELEASE", 540.0f) },
          { 1, 2, 2, 1, 1, 2, 2 },
          { { 0.28f, 0.26f, 0.12f, 0.60f }, 0.64f, 0.12f, 0.22f, 0.24f } },

        { "Pad - Vintage Wash",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.58f),
            makeParam ("attack", 0.75f), makeParam ("decay", 1.10f), makeParam ("sustain", 0.82f), makeParam ("release", 2.60f),
            makeParam ("glide", 0.06f), makeParam ("drive", 1.8f), makeParam ("filterCutoff", 1500.0f),
            makeParam ("CHORUS_AMOUNT", 0.66f), makeParam ("CHORUS_MIX", 0.34f),
            makeParam ("DELAY_AMOUNT", 0.48f), makeParam ("DELAY_MIX", 0.16f),
            makeParam ("PHASER_AMOUNT", 0.24f), makeParam ("PHASER_MIX", 0.10f),
            makeParam ("REVERB_AMOUNT", 0.66f), makeParam ("REVERB_MIX", 0.34f),
            makeParam ("LOW_GAIN", 4.0f), makeParam ("MID_GAIN", -5.0f), makeParam ("HIGH_GAIN", -1.0f),
            makeParam ("MASTER_GAIN", -1.0f),
            makeParam ("COMP_THRESHOLD", -14.0f), makeParam ("COMP_RATIO", 2.2f), makeParam ("COMP_ATTACK", 36.0f), makeParam ("COMP_RELEASE", 300.0f) },
          { 1, 2, 1, 2, 3, 3, 1 },
          { { 0.20f, 0.22f, 0.16f, 0.48f }, 0.36f, 0.16f, 0.14f, 0.12f } },

        { "Sine - 808 Tunnel",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.12f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.62f), makeParam ("sustain", 0.88f), makeParam ("release", 0.78f),
            makeParam ("glide", 0.34f), makeParam ("drive", 14.5f), makeParam ("filterCutoff", 180.0f),
            makeParam ("CHORUS_AMOUNT", 0.02f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.10f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.04f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.05f), makeParam ("REVERB_MIX", 0.00f),
            makeParam ("LOW_GAIN", 14.0f), makeParam ("MID_GAIN", -12.0f), makeParam ("HIGH_GAIN", -20.0f),
            makeParam ("MASTER_GAIN", -1.0f),
            makeParam ("COMP_THRESHOLD", -12.0f), makeParam ("COMP_RATIO", 6.5f), makeParam ("COMP_ATTACK", 8.0f), makeParam ("COMP_RELEASE", 260.0f) },
          { 3, 2, 2, 3, 1, 1, 1 },
          { { 0.02f, 0.00f, 0.00f, 0.04f }, 0.18f, 0.46f, 0.04f, 0.01f } },

        { "Sine - Ice Glock",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.54f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.05f), makeParam ("sustain", 0.00f), makeParam ("release", 0.11f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.4f), makeParam ("filterCutoff", 14500.0f),
            makeParam ("CHORUS_AMOUNT", 0.36f), makeParam ("CHORUS_MIX", 0.20f),
            makeParam ("DELAY_AMOUNT", 0.36f), makeParam ("DELAY_MIX", 0.18f),
            makeParam ("PHASER_AMOUNT", 0.18f), makeParam ("PHASER_MIX", 0.07f),
            makeParam ("REVERB_AMOUNT", 0.72f), makeParam ("REVERB_MIX", 0.36f),
            makeParam ("LOW_GAIN", -16.0f), makeParam ("MID_GAIN", -4.0f), makeParam ("HIGH_GAIN", 14.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -18.0f), makeParam ("COMP_RATIO", 3.6f), makeParam ("COMP_ATTACK", 1.5f), makeParam ("COMP_RELEASE", 95.0f) },
          { 1, 3, 2, 1, 3, 2, 2 },
          { { 0.24f, 0.22f, 0.10f, 0.48f }, 0.92f, 0.12f, 0.22f, 0.10f } },

        { "Sine - Whistle Flex",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.24f),
            makeParam ("attack", 0.004f), makeParam ("decay", 0.14f), makeParam ("sustain", 0.64f), makeParam ("release", 0.24f),
            makeParam ("glide", 0.30f), makeParam ("drive", 4.2f), makeParam ("filterCutoff", 13500.0f),
            makeParam ("CHORUS_AMOUNT", 0.16f), makeParam ("CHORUS_MIX", 0.05f),
            makeParam ("DELAY_AMOUNT", 0.52f), makeParam ("DELAY_MIX", 0.24f),
            makeParam ("PHASER_AMOUNT", 0.08f), makeParam ("PHASER_MIX", 0.02f),
            makeParam ("REVERB_AMOUNT", 0.42f), makeParam ("REVERB_MIX", 0.18f),
            makeParam ("LOW_GAIN", -18.0f), makeParam ("MID_GAIN", -1.0f), makeParam ("HIGH_GAIN", 11.0f),
            makeParam ("MASTER_GAIN", -1.0f),
            makeParam ("COMP_THRESHOLD", -13.0f), makeParam ("COMP_RATIO", 2.6f), makeParam ("COMP_ATTACK", 7.0f), makeParam ("COMP_RELEASE", 120.0f) },
          { 2, 3, 1, 2, 3, 2, 1 },
          { { 0.14f, 0.40f, 0.05f, 0.32f }, 0.90f, 0.14f, 0.34f, 0.18f } },

        { "Sine - Deep Organ",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.44f),
            makeParam ("attack", 0.018f), makeParam ("decay", 0.40f), makeParam ("sustain", 0.76f), makeParam ("release", 0.86f),
            makeParam ("glide", 0.08f), makeParam ("drive", 5.0f), makeParam ("filterCutoff", 1600.0f),
            makeParam ("CHORUS_AMOUNT", 0.20f), makeParam ("CHORUS_MIX", 0.08f),
            makeParam ("DELAY_AMOUNT", 0.28f), makeParam ("DELAY_MIX", 0.08f),
            makeParam ("PHASER_AMOUNT", 0.22f), makeParam ("PHASER_MIX", 0.10f),
            makeParam ("REVERB_AMOUNT", 0.44f), makeParam ("REVERB_MIX", 0.18f),
            makeParam ("LOW_GAIN", 7.0f), makeParam ("MID_GAIN", -5.0f), makeParam ("HIGH_GAIN", -4.0f),
            makeParam ("MASTER_GAIN", -2.0f),
            makeParam ("COMP_THRESHOLD", -15.0f), makeParam ("COMP_RATIO", 3.2f), makeParam ("COMP_ATTACK", 22.0f), makeParam ("COMP_RELEASE", 210.0f) },
          { 1, 2, 1, 2, 2, 3, 2 },
          { { 0.18f, 0.12f, 0.16f, 0.34f }, 0.38f, 0.18f, 0.22f, 0.12f } },

        { "Sine - Sub Bell Pad",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.36f),
            makeParam ("attack", 0.62f), makeParam ("decay", 1.20f), makeParam ("sustain", 0.72f), makeParam ("release", 3.40f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.6f), makeParam ("filterCutoff", 5200.0f),
            makeParam ("CHORUS_AMOUNT", 0.58f), makeParam ("CHORUS_MIX", 0.34f),
            makeParam ("DELAY_AMOUNT", 0.62f), makeParam ("DELAY_MIX", 0.20f),
            makeParam ("PHASER_AMOUNT", 0.12f), makeParam ("PHASER_MIX", 0.04f),
            makeParam ("REVERB_AMOUNT", 0.86f), makeParam ("REVERB_MIX", 0.46f),
            makeParam ("LOW_GAIN", 3.0f), makeParam ("MID_GAIN", -4.0f), makeParam ("HIGH_GAIN", 6.0f),
            makeParam ("MASTER_GAIN", -1.0f),
            makeParam ("COMP_THRESHOLD", -10.0f), makeParam ("COMP_RATIO", 1.7f), makeParam ("COMP_ATTACK", 70.0f), makeParam ("COMP_RELEASE", 560.0f) },
          { 1, 2, 1, 1, 3, 2, 1 },
          { { 0.28f, 0.24f, 0.08f, 0.54f }, 0.58f, 0.12f, 0.18f, 0.14f } },

        { "Square - Rage Reese",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 1.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 0.003f), makeParam ("decay", 0.20f), makeParam ("sustain", 0.58f), makeParam ("release", 0.26f),
            makeParam ("glide", 0.20f), makeParam ("drive", 17.0f), makeParam ("filterCutoff", 520.0f),
            makeParam ("CHORUS_AMOUNT", 0.28f), makeParam ("CHORUS_MIX", 0.18f),
            makeParam ("DELAY_AMOUNT", 0.16f), makeParam ("DELAY_MIX", 0.02f),
            makeParam ("PHASER_AMOUNT", 0.44f), makeParam ("PHASER_MIX", 0.24f),
            makeParam ("REVERB_AMOUNT", 0.10f), makeParam ("REVERB_MIX", 0.02f),
            makeParam ("LOW_GAIN", 8.0f), makeParam ("MID_GAIN", 9.0f), makeParam ("HIGH_GAIN", -6.0f),
            makeParam ("MASTER_GAIN", -4.0f),
            makeParam ("COMP_THRESHOLD", -22.0f), makeParam ("COMP_RATIO", 8.0f), makeParam ("COMP_ATTACK", 2.0f), makeParam ("COMP_RELEASE", 70.0f) },
          { 3, 2, 1, 3, 2, 3, 3 },
          { { 0.26f, 0.04f, 0.30f, 0.10f }, 0.31f, 0.52f, 0.45f, 0.30f } },

        { "Square - Dark Plonk",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 1.0f), makeParam ("oscmix", 0.58f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.08f), makeParam ("sustain", 0.00f), makeParam ("release", 0.12f),
            makeParam ("glide", 0.00f), makeParam ("drive", 8.0f), makeParam ("filterCutoff", 740.0f),
            makeParam ("CHORUS_AMOUNT", 0.08f), makeParam ("CHORUS_MIX", 0.02f),
            makeParam ("DELAY_AMOUNT", 0.20f), makeParam ("DELAY_MIX", 0.06f),
            makeParam ("PHASER_AMOUNT", 0.36f), makeParam ("PHASER_MIX", 0.16f),
            makeParam ("REVERB_AMOUNT", 0.18f), makeParam ("REVERB_MIX", 0.06f),
            makeParam ("LOW_GAIN", 2.0f), makeParam ("MID_GAIN", 8.0f), makeParam ("HIGH_GAIN", -8.0f),
            makeParam ("MASTER_GAIN", -2.0f),
            makeParam ("COMP_THRESHOLD", -20.0f), makeParam ("COMP_RATIO", 5.5f), makeParam ("COMP_ATTACK", 1.0f), makeParam ("COMP_RELEASE", 55.0f) },
          { 1, 2, 2, 2, 1, 3, 3 },
          { { 0.08f, 0.12f, 0.26f, 0.16f }, 0.28f, 0.24f, 0.32f, 0.22f } },

        { "Square - Bit Lead",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 1.0f), makeParam ("oscmix", 0.62f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.07f), makeParam ("sustain", 0.80f), makeParam ("release", 0.10f),
            makeParam ("glide", 0.38f), makeParam ("drive", 11.0f), makeParam ("filterCutoff", 8200.0f),
            makeParam ("CHORUS_AMOUNT", 0.06f), makeParam ("CHORUS_MIX", 0.02f),
            makeParam ("DELAY_AMOUNT", 0.38f), makeParam ("DELAY_MIX", 0.20f),
            makeParam ("PHASER_AMOUNT", 0.18f), makeParam ("PHASER_MIX", 0.08f),
            makeParam ("REVERB_AMOUNT", 0.14f), makeParam ("REVERB_MIX", 0.04f),
            makeParam ("LOW_GAIN", -10.0f), makeParam ("MID_GAIN", 7.0f), makeParam ("HIGH_GAIN", 10.0f),
            makeParam ("MASTER_GAIN", -3.0f),
            makeParam ("COMP_THRESHOLD", -17.0f), makeParam ("COMP_RATIO", 4.2f), makeParam ("COMP_ATTACK", 1.0f), makeParam ("COMP_RELEASE", 60.0f) },
          { 3, 3, 1, 2, 1, 2, 3 },
          { { 0.06f, 0.28f, 0.12f, 0.10f }, 0.78f, 0.20f, 0.44f, 0.28f } },

        { "Square - Haunted Keys",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 1.0f), makeParam ("oscmix", 0.46f),
            makeParam ("attack", 0.012f), makeParam ("decay", 0.30f), makeParam ("sustain", 0.32f), makeParam ("release", 0.58f),
            makeParam ("glide", 0.03f), makeParam ("drive", 4.8f), makeParam ("filterCutoff", 2600.0f),
            makeParam ("CHORUS_AMOUNT", 0.34f), makeParam ("CHORUS_MIX", 0.20f),
            makeParam ("DELAY_AMOUNT", 0.48f), makeParam ("DELAY_MIX", 0.18f),
            makeParam ("PHASER_AMOUNT", 0.28f), makeParam ("PHASER_MIX", 0.12f),
            makeParam ("REVERB_AMOUNT", 0.54f), makeParam ("REVERB_MIX", 0.24f),
            makeParam ("LOW_GAIN", -5.0f), makeParam ("MID_GAIN", 3.0f), makeParam ("HIGH_GAIN", 5.0f),
            makeParam ("MASTER_GAIN", -1.0f),
            makeParam ("COMP_THRESHOLD", -16.0f), makeParam ("COMP_RATIO", 3.0f), makeParam ("COMP_ATTACK", 12.0f), makeParam ("COMP_RELEASE", 170.0f) },
          { 1, 2, 1, 1, 2, 3, 2 },
          { { 0.24f, 0.24f, 0.18f, 0.40f }, 0.46f, 0.18f, 0.24f, 0.16f } },

        { "Square - Plug Chord",
          { makeParam ("osc1type", 1.0f), makeParam ("osc2type", 1.0f), makeParam ("oscmix", 0.52f),
            makeParam ("attack", 0.005f), makeParam ("decay", 0.16f), makeParam ("sustain", 0.12f), makeParam ("release", 0.28f),
            makeParam ("glide", 0.00f), makeParam ("drive", 3.2f), makeParam ("filterCutoff", 5200.0f),
            makeParam ("CHORUS_AMOUNT", 0.40f), makeParam ("CHORUS_MIX", 0.22f),
            makeParam ("DELAY_AMOUNT", 0.56f), makeParam ("DELAY_MIX", 0.24f),
            makeParam ("PHASER_AMOUNT", 0.10f), makeParam ("PHASER_MIX", 0.02f),
            makeParam ("REVERB_AMOUNT", 0.42f), makeParam ("REVERB_MIX", 0.18f),
            makeParam ("LOW_GAIN", -7.0f), makeParam ("MID_GAIN", 4.0f), makeParam ("HIGH_GAIN", 7.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -15.0f), makeParam ("COMP_RATIO", 3.2f), makeParam ("COMP_ATTACK", 6.0f), makeParam ("COMP_RELEASE", 110.0f) },
          { 1, 2, 1, 1, 3, 1, 1 },
          { { 0.30f, 0.30f, 0.06f, 0.30f }, 0.58f, 0.18f, 0.14f, 0.08f } },

        { "Saw - Carti Siren",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.64f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.06f), makeParam ("sustain", 0.82f), makeParam ("release", 0.09f),
            makeParam ("glide", 0.52f), makeParam ("drive", 13.0f), makeParam ("filterCutoff", 11000.0f),
            makeParam ("CHORUS_AMOUNT", 0.08f), makeParam ("CHORUS_MIX", 0.02f),
            makeParam ("DELAY_AMOUNT", 0.46f), makeParam ("DELAY_MIX", 0.30f),
            makeParam ("PHASER_AMOUNT", 0.20f), makeParam ("PHASER_MIX", 0.08f),
            makeParam ("REVERB_AMOUNT", 0.18f), makeParam ("REVERB_MIX", 0.06f),
            makeParam ("LOW_GAIN", -12.0f), makeParam ("MID_GAIN", 5.0f), makeParam ("HIGH_GAIN", 12.0f),
            makeParam ("MASTER_GAIN", -4.0f),
            makeParam ("COMP_THRESHOLD", -16.0f), makeParam ("COMP_RATIO", 3.8f), makeParam ("COMP_ATTACK", 1.0f), makeParam ("COMP_RELEASE", 55.0f) },
          { 3, 3, 1, 2, 1, 2, 3 },
          { { 0.08f, 0.36f, 0.12f, 0.14f }, 0.84f, 0.18f, 0.58f, 0.34f } },

        { "Saw - Rage Stack",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.72f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.11f), makeParam ("sustain", 0.66f), makeParam ("release", 0.18f),
            makeParam ("glide", 0.12f), makeParam ("drive", 15.0f), makeParam ("filterCutoff", 1250.0f),
            makeParam ("CHORUS_AMOUNT", 0.14f), makeParam ("CHORUS_MIX", 0.06f),
            makeParam ("DELAY_AMOUNT", 0.28f), makeParam ("DELAY_MIX", 0.10f),
            makeParam ("PHASER_AMOUNT", 0.64f), makeParam ("PHASER_MIX", 0.28f),
            makeParam ("REVERB_AMOUNT", 0.16f), makeParam ("REVERB_MIX", 0.05f),
            makeParam ("LOW_GAIN", -6.0f), makeParam ("MID_GAIN", 10.0f), makeParam ("HIGH_GAIN", 7.0f),
            makeParam ("MASTER_GAIN", -5.0f),
            makeParam ("COMP_THRESHOLD", -20.0f), makeParam ("COMP_RATIO", 7.5f), makeParam ("COMP_ATTACK", 1.0f), makeParam ("COMP_RELEASE", 60.0f) },
          { 3, 2, 1, 3, 1, 3, 3 },
          { { 0.12f, 0.16f, 0.34f, 0.12f }, 0.56f, 0.42f, 0.48f, 0.30f } },

        { "Saw - Metro Keys",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.42f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.12f), makeParam ("sustain", 0.00f), makeParam ("release", 0.18f),
            makeParam ("glide", 0.00f), makeParam ("drive", 3.6f), makeParam ("filterCutoff", 7600.0f),
            makeParam ("CHORUS_AMOUNT", 0.22f), makeParam ("CHORUS_MIX", 0.10f),
            makeParam ("DELAY_AMOUNT", 0.58f), makeParam ("DELAY_MIX", 0.32f),
            makeParam ("PHASER_AMOUNT", 0.06f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.34f), makeParam ("REVERB_MIX", 0.16f),
            makeParam ("LOW_GAIN", -10.0f), makeParam ("MID_GAIN", 2.0f), makeParam ("HIGH_GAIN", 9.0f),
            makeParam ("MASTER_GAIN", 1.0f),
            makeParam ("COMP_THRESHOLD", -15.0f), makeParam ("COMP_RATIO", 3.0f), makeParam ("COMP_ATTACK", 4.0f), makeParam ("COMP_RELEASE", 75.0f) },
          { 1, 2, 1, 1, 2, 1, 1 },
          { { 0.18f, 0.34f, 0.04f, 0.24f }, 0.66f, 0.20f, 0.12f, 0.04f } },

        { "Saw - Space Strings",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.60f),
            makeParam ("attack", 1.40f), makeParam ("decay", 2.40f), makeParam ("sustain", 0.74f), makeParam ("release", 3.60f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.3f), makeParam ("filterCutoff", 4200.0f),
            makeParam ("CHORUS_AMOUNT", 0.84f), makeParam ("CHORUS_MIX", 0.46f),
            makeParam ("DELAY_AMOUNT", 0.70f), makeParam ("DELAY_MIX", 0.24f),
            makeParam ("PHASER_AMOUNT", 0.22f), makeParam ("PHASER_MIX", 0.08f),
            makeParam ("REVERB_AMOUNT", 0.88f), makeParam ("REVERB_MIX", 0.48f),
            makeParam ("LOW_GAIN", -2.0f), makeParam ("MID_GAIN", -3.0f), makeParam ("HIGH_GAIN", 6.0f),
            makeParam ("MASTER_GAIN", -1.0f),
            makeParam ("COMP_THRESHOLD", -9.0f), makeParam ("COMP_RATIO", 1.5f), makeParam ("COMP_ATTACK", 90.0f), makeParam ("COMP_RELEASE", 700.0f) },
          { 1, 2, 1, 1, 2, 2, 1 },
          { { 0.36f, 0.28f, 0.12f, 0.58f }, 0.54f, 0.10f, 0.20f, 0.16f } },

        { "Saw - Trap Brass",
          { makeParam ("osc1type", 2.0f), makeParam ("osc2type", 2.0f), makeParam ("oscmix", 0.68f),
            makeParam ("attack", 0.006f), makeParam ("decay", 0.24f), makeParam ("sustain", 0.70f), makeParam ("release", 0.34f),
            makeParam ("glide", 0.10f), makeParam ("drive", 9.0f), makeParam ("filterCutoff", 2400.0f),
            makeParam ("CHORUS_AMOUNT", 0.18f), makeParam ("CHORUS_MIX", 0.08f),
            makeParam ("DELAY_AMOUNT", 0.30f), makeParam ("DELAY_MIX", 0.10f),
            makeParam ("PHASER_AMOUNT", 0.12f), makeParam ("PHASER_MIX", 0.04f),
            makeParam ("REVERB_AMOUNT", 0.28f), makeParam ("REVERB_MIX", 0.10f),
            makeParam ("LOW_GAIN", -4.0f), makeParam ("MID_GAIN", 8.0f), makeParam ("HIGH_GAIN", 5.0f),
            makeParam ("MASTER_GAIN", -3.0f),
            makeParam ("COMP_THRESHOLD", -18.0f), makeParam ("COMP_RATIO", 5.0f), makeParam ("COMP_ATTACK", 4.0f), makeParam ("COMP_RELEASE", 90.0f) },
          { 3, 2, 1, 3, 1, 1, 2 },
          { { 0.12f, 0.14f, 0.08f, 0.18f }, 0.48f, 0.30f, 0.20f, 0.16f } },

        { "Drum - Street Kit",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.30f), makeParam ("sustain", 0.70f), makeParam ("release", 0.40f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.0f), makeParam ("filterCutoff", 16000.0f),
            makeParam ("CHORUS_AMOUNT", 0.00f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.25f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.00f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.10f), makeParam ("REVERB_MIX", 0.00f),
            makeParam ("LOW_GAIN", 3.0f), makeParam ("MID_GAIN", 0.0f), makeParam ("HIGH_GAIN", 0.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -12.0f), makeParam ("COMP_RATIO", 4.0f), makeParam ("COMP_ATTACK", 10.0f), makeParam ("COMP_RELEASE", 100.0f) },
          { 1, 1, 1, 1, 1, 1, 1 },
          { { 0.00f, 0.00f, 0.00f, 0.10f }, 1.00f, 0.20f, 0.10f, 0.00f }, 0 },

        { "Drum - Charged Kit",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.30f), makeParam ("sustain", 0.70f), makeParam ("release", 0.40f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.0f), makeParam ("filterCutoff", 16000.0f),
            makeParam ("CHORUS_AMOUNT", 0.00f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.25f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.00f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.10f), makeParam ("REVERB_MIX", 0.00f),
            makeParam ("LOW_GAIN", 3.0f), makeParam ("MID_GAIN", 0.0f), makeParam ("HIGH_GAIN", 0.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -12.0f), makeParam ("COMP_RATIO", 4.0f), makeParam ("COMP_ATTACK", 10.0f), makeParam ("COMP_RELEASE", 100.0f) },
          { 1, 1, 1, 1, 1, 1, 1 },
          { { 0.00f, 0.00f, 0.00f, 0.10f }, 1.00f, 0.20f, 0.10f, 0.00f }, 1 },

        { "Drum - Damage Kit",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.30f), makeParam ("sustain", 0.70f), makeParam ("release", 0.40f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.0f), makeParam ("filterCutoff", 16000.0f),
            makeParam ("CHORUS_AMOUNT", 0.00f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.25f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.00f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.10f), makeParam ("REVERB_MIX", 0.00f),
            makeParam ("LOW_GAIN", 3.0f), makeParam ("MID_GAIN", 0.0f), makeParam ("HIGH_GAIN", 0.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -12.0f), makeParam ("COMP_RATIO", 4.0f), makeParam ("COMP_ATTACK", 10.0f), makeParam ("COMP_RELEASE", 100.0f) },
          { 1, 1, 1, 1, 1, 1, 1 },
          { { 0.00f, 0.00f, 0.00f, 0.10f }, 1.00f, 0.20f, 0.10f, 0.00f }, 2 },

        { "Drum - Mobb Kit",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.30f), makeParam ("sustain", 0.70f), makeParam ("release", 0.40f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.0f), makeParam ("filterCutoff", 16000.0f),
            makeParam ("CHORUS_AMOUNT", 0.00f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.25f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.00f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.10f), makeParam ("REVERB_MIX", 0.00f),
            makeParam ("LOW_GAIN", 3.0f), makeParam ("MID_GAIN", 0.0f), makeParam ("HIGH_GAIN", 0.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -12.0f), makeParam ("COMP_RATIO", 4.0f), makeParam ("COMP_ATTACK", 10.0f), makeParam ("COMP_RELEASE", 100.0f) },
          { 1, 1, 1, 1, 1, 1, 1 },
          { { 0.00f, 0.00f, 0.00f, 0.10f }, 1.00f, 0.20f, 0.10f, 0.00f }, 3 },

        { "Drum - Spinzy Kit",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.30f), makeParam ("sustain", 0.70f), makeParam ("release", 0.40f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.0f), makeParam ("filterCutoff", 16000.0f),
            makeParam ("CHORUS_AMOUNT", 0.00f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.25f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.00f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.10f), makeParam ("REVERB_MIX", 0.00f),
            makeParam ("LOW_GAIN", 3.0f), makeParam ("MID_GAIN", 0.0f), makeParam ("HIGH_GAIN", 0.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -12.0f), makeParam ("COMP_RATIO", 4.0f), makeParam ("COMP_ATTACK", 10.0f), makeParam ("COMP_RELEASE", 100.0f) },
          { 1, 1, 1, 1, 1, 1, 1 },
          { { 0.00f, 0.00f, 0.00f, 0.10f }, 1.00f, 0.20f, 0.10f, 0.00f }, 4 },

        { "Drum - Tripp Kit",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.30f), makeParam ("sustain", 0.70f), makeParam ("release", 0.40f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.0f), makeParam ("filterCutoff", 16000.0f),
            makeParam ("CHORUS_AMOUNT", 0.00f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.25f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.00f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.10f), makeParam ("REVERB_MIX", 0.00f),
            makeParam ("LOW_GAIN", 3.0f), makeParam ("MID_GAIN", 0.0f), makeParam ("HIGH_GAIN", 0.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -12.0f), makeParam ("COMP_RATIO", 4.0f), makeParam ("COMP_ATTACK", 10.0f), makeParam ("COMP_RELEASE", 100.0f) },
          { 1, 1, 1, 1, 1, 1, 1 },
          { { 0.00f, 0.00f, 0.00f, 0.10f }, 1.00f, 0.20f, 0.10f, 0.00f }, 5 },

        { "Drum - Zayy Kit",
          { makeParam ("osc1type", 0.0f), makeParam ("osc2type", 0.0f), makeParam ("oscmix", 0.50f),
            makeParam ("attack", 0.001f), makeParam ("decay", 0.30f), makeParam ("sustain", 0.70f), makeParam ("release", 0.40f),
            makeParam ("glide", 0.00f), makeParam ("drive", 1.0f), makeParam ("filterCutoff", 16000.0f),
            makeParam ("CHORUS_AMOUNT", 0.00f), makeParam ("CHORUS_MIX", 0.00f),
            makeParam ("DELAY_AMOUNT", 0.25f), makeParam ("DELAY_MIX", 0.00f),
            makeParam ("PHASER_AMOUNT", 0.00f), makeParam ("PHASER_MIX", 0.00f),
            makeParam ("REVERB_AMOUNT", 0.10f), makeParam ("REVERB_MIX", 0.00f),
            makeParam ("LOW_GAIN", 3.0f), makeParam ("MID_GAIN", 0.0f), makeParam ("HIGH_GAIN", 0.0f),
            makeParam ("MASTER_GAIN", 0.0f),
            makeParam ("COMP_THRESHOLD", -12.0f), makeParam ("COMP_RATIO", 4.0f), makeParam ("COMP_ATTACK", 10.0f), makeParam ("COMP_RELEASE", 100.0f) },
          { 1, 1, 1, 1, 1, 1, 1 },
          { { 0.00f, 0.00f, 0.00f, 0.10f }, 1.00f, 0.20f, 0.10f, 0.00f }, 6 }
    }};

    int countWhiteKeysInRange (int lowNote, int highNote)
    {
        int count = 0;
        for (int note = lowNote; note <= highNote; ++note)
        {
            switch (note % 12)
            {
                case 1: case 3: case 6: case 8: case 10:
                    break;
                default:
                    ++count;
                    break;
            }
        }
        return count;
    }

    constexpr int userPresetIdBase = 1000;
    constexpr int savePresetItemId = 9001;
    constexpr int loadPresetItemId = 9002;
    constexpr int refreshPresetsItemId = 9003;

    juce::String makeSafePresetFileName (juce::String name)
    {
        name = name.trim();
        if (name.isEmpty())
            name = "Killer B Preset";

        for (auto c : juce::String ("\\/:*?\"<>|"))
            name = name.replaceCharacter (c, '-');

        return name;
    }

    struct FileNameComparator
    {
        static int compareElements (const juce::File& a, const juce::File& b)
        {
            return a.getFileName().compareIgnoreCase (b.getFileName());
        }
    };
}

float KillaBEditor::calculateDisplayFitScale() const
{
    const auto& displays = juce::Desktop::getInstance().getDisplays();
    juce::Rectangle<int> availableArea;

    if (auto* mouseDisplay = displays.getDisplayForPoint (juce::Desktop::getMousePosition()))
        availableArea = mouseDisplay->userArea;

    if (availableArea.isEmpty())
        if (auto* primaryDisplay = displays.getPrimaryDisplay())
            availableArea = primaryDisplay->userArea;

    if (availableArea.isEmpty())
        return 1.0f;

    const auto usableWidth = juce::jmax (1, availableArea.getWidth() - screenFitMargin);
    const auto usableHeight = juce::jmax (1, availableArea.getHeight() - screenFitMargin);
    const auto fitScale = juce::jmin ((float) usableWidth / (float) designWidth,
                                      (float) usableHeight / (float) designHeight);

    return juce::jlimit (minimumDisplayFitScale, 1.0f, fitScale);
}

void KillaBEditor::setScaleFactor (float newScale)
{
    AudioProcessorEditor::setScaleFactor (newScale * displayFitScale);
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
        const float y = b.getCentreY() - std::tanh (s * drive) * normFactor * b.getHeight() * 0.33f;
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
    displayFitScale = calculateDisplayFitScale();
    setSize (designWidth, designHeight);
    setScaleFactor (1.0f);

    topLeftLogoImage = loadBinaryImageIfPresent ("KILLA_B_png");
    centerBrandImage = loadBinaryImageIfPresent ("TURNE_ME_UP_png");
    beeImage = loadBinaryImageIfPresent ("GOLD_BEE_png");

    setupCombo (presetCombo, presetTitleLabel, "PRESETS");
    populateFactoryPresets();

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
    filterTypeAttach = std::make_unique<CA> (processorRef.apvts, "filterType", filterTypeBox);

    setupCombo (filterSlopeBox, filterSlopeLabel, "Filter slope");
    filterSlopeBox.addItem ("12 dB/oct", 1);
    filterSlopeBox.addItem ("24 dB/oct", 2);
    filterSlopeBox.setSelectedId (1, juce::dontSendNotification);
    filterSlopeAttach = std::make_unique<CA> (processorRef.apvts, "filterSlope", filterSlopeBox);

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
    masterGainKnob.getProperties().set ("kbUseGainFilmstrip", true);
    masterGainAttach = std::make_unique<SA> (processorRef.apvts, "MASTER_GAIN", masterGainKnob);

    setupKnob (glideKnob, glideLabel, "glide", "gold");
    glideAttach = std::make_unique<SA> (processorRef.apvts, "glide", glideKnob);

    setupPassiveKnob (cutoffKnob, cutoffLabel, "filt. cut off");
    cutoffAttach = std::make_unique<SA> (processorRef.apvts, "filterCutoff", cutoffKnob);

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

    keyboard.setAvailableRange (24, 108);
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
    slider.getProperties().set ("kbAdsrSlider", true);
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

void KillaBEditor::populateFactoryPresets()
{
    presetCombo.clear (juce::dontSendNotification);
    presetCombo.setTextWhenNothingSelected ("Choose preset");
    presetCombo.setJustificationType (juce::Justification::centredLeft);
    refreshUserPresetFiles();

    auto displayCategory = [] (juce::String category)
    {
        category = category.trim().toLowerCase();

        if (category == "bass")  return juce::String ("BASSES");
        if (category == "lead")  return juce::String ("LEADS");
        if (category == "pluck") return juce::String ("PLUCKS");
        if (category == "synth") return juce::String ("SYNTHS");
        if (category == "pad")   return juce::String ("PADS");
        if (category == "drum")  return juce::String ("DRUM KITS");

        return category.toUpperCase();
    };

    juce::String currentCategory;
    for (size_t i = 0; i < factoryPresets.size(); ++i)
    {
        const juce::String presetName (factoryPresets[i].name);
        const auto category = presetName.upToFirstOccurrenceOf (" - ", false, false);

        if (category != currentCategory)
        {
            if (currentCategory.isNotEmpty())
                presetCombo.addSeparator();

            presetCombo.addSectionHeading (displayCategory (category));
            currentCategory = category;
        }

        presetCombo.addItem (presetName.fromFirstOccurrenceOf (" - ", false, false), (int) i + 1);
    }

    if (! userPresetFiles.isEmpty())
    {
        presetCombo.addSeparator();
        presetCombo.addSectionHeading ("USER PRESETS");

        for (int i = 0; i < userPresetFiles.size(); ++i)
            presetCombo.addItem (userPresetFiles.getReference (i).getFileNameWithoutExtension(), userPresetIdBase + i);
    }

    presetCombo.addSeparator();
    presetCombo.addItem ("Save Current Preset...", savePresetItemId);
    presetCombo.addItem ("Load Preset File...", loadPresetItemId);
    presetCombo.addItem ("Refresh User Presets", refreshPresetsItemId);

    presetCombo.onChange = [this]
    {
        if (isApplyingFactoryPreset)
            return;

        const int selectedId = presetCombo.getSelectedId();
        const int factoryPresetIndex = selectedId - 1;
        const int userPresetIndex = selectedId - userPresetIdBase;

        if (juce::isPositiveAndBelow (factoryPresetIndex, (int) factoryPresets.size()))
        {
            applyFactoryPreset (factoryPresetIndex);
            return;
        }

        if (juce::isPositiveAndBelow (userPresetIndex, userPresetFiles.size()))
        {
            applyUserPreset (userPresetIndex);
            return;
        }

        if (selectedId == savePresetItemId)
        {
            launchPresetSaveChooser();
            return;
        }

        if (selectedId == loadPresetItemId)
        {
            launchPresetLoadChooser();
            return;
        }

        if (selectedId == refreshPresetsItemId)
        {
            populateFactoryPresets();
            resetPresetSelection();
        }
    };
}

void KillaBEditor::refreshUserPresetFiles()
{
    userPresetFiles.clear();

    auto presetDirectory = KillerBProcessor::getUserPresetDirectory();
    presetDirectory.createDirectory();

    juce::Array<juce::File> files;
    presetDirectory.findChildFiles (files, juce::File::findFiles, false, "*.kbpreset");
    presetDirectory.findChildFiles (files, juce::File::findFiles, false, "*.xml");
    FileNameComparator comparator;
    files.sort (comparator);

    for (const auto& file : files)
        if (! userPresetFiles.contains (file))
            userPresetFiles.add (file);
}

void KillaBEditor::applyFactoryPreset (int presetIndex)
{
    if (! juce::isPositiveAndBelow (presetIndex, (int) factoryPresets.size()))
        return;

    const auto& preset = factoryPresets[(size_t) presetIndex];
    juce::ScopedValueSetter<bool> applyingPreset (isApplyingFactoryPreset, true);

    for (const auto& param : preset.params)
        if (auto* ranged = processorRef.apvts.getParameter (param.id))
            ranged->setValueNotifyingHost (ranged->convertTo0to1 (param.value));

    catCombo.setSelectedId (preset.combos.modeId, juce::sendNotificationSync);
    filterTypeBox.setSelectedId (preset.combos.filterTypeId, juce::sendNotificationSync);
    filterSlopeBox.setSelectedId (preset.combos.filterSlopeId, juce::sendNotificationSync);
    voicesBox.setSelectedId (preset.combos.voicesId, juce::sendNotificationSync);
    reverbPresetBox.setSelectedId (preset.combos.reverbPresetId, juce::sendNotificationSync);
    lfoRoutingBox.setSelectedId (preset.combos.lfoRoutingId, juce::sendNotificationSync);
    lfoTypeBox.setSelectedId (preset.combos.lfoTypeId, juce::sendNotificationSync);

    for (int i = 0; i < 4; ++i)
        fxDetailKnobs[i].setValue (preset.uiKnobs.fxDetail[i], juce::sendNotificationSync);

    cutoffKnob.setValue (preset.uiKnobs.cutoff, juce::sendNotificationSync);
    compMixKnob.setValue (preset.uiKnobs.compMix, juce::sendNotificationSync);
    lfoRateKnob.setValue (preset.uiKnobs.lfoRate, juce::sendNotificationSync);
    lfoDepthKnob.setValue (preset.uiKnobs.lfoDepth, juce::sendNotificationSync);
    processorRef.setActiveDrumKit (preset.drumKitId);

    presetCombo.setSelectedId (presetIndex + 1, juce::dontSendNotification);
}

void KillaBEditor::applyUserPreset (int presetIndex)
{
    if (! juce::isPositiveAndBelow (presetIndex, userPresetFiles.size()))
        return;

    juce::ScopedValueSetter<bool> applyingPreset (isApplyingFactoryPreset, true);

    if (processorRef.loadPresetFromFile (userPresetFiles.getReference (presetIndex)))
        presetCombo.setSelectedId (userPresetIdBase + presetIndex, juce::dontSendNotification);
    else
        resetPresetSelection();
}

void KillaBEditor::launchPresetLoadChooser()
{
    resetPresetSelection();

    presetFileChooser = std::make_unique<juce::FileChooser> (
        "Load Killer B preset",
        KillerBProcessor::getUserPresetDirectory(),
        "*.kbpreset;*.xml");

    juce::Component::SafePointer<KillaBEditor> safeThis (this);
    presetFileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                    [safeThis] (const juce::FileChooser& chooser)
    {
        if (safeThis == nullptr)
            return;

        const auto file = chooser.getResult();
        if (file.existsAsFile())
        {
            safeThis->processorRef.loadPresetFromFile (file);
            safeThis->populateFactoryPresets();
        }

        safeThis->resetPresetSelection();
        safeThis->presetFileChooser.reset();
    });
}

void KillaBEditor::launchPresetSaveChooser()
{
    resetPresetSelection();

    const auto directory = KillerBProcessor::getUserPresetDirectory();
    directory.createDirectory();

    presetFileChooser = std::make_unique<juce::FileChooser> (
        "Save Killer B preset",
        directory.getChildFile ("Killer B Preset.kbpreset"),
        "*.kbpreset");

    juce::Component::SafePointer<KillaBEditor> safeThis (this);
    presetFileChooser->launchAsync (juce::FileBrowserComponent::saveMode
                                        | juce::FileBrowserComponent::canSelectFiles
                                        | juce::FileBrowserComponent::warnAboutOverwriting,
                                    [safeThis] (const juce::FileChooser& chooser)
    {
        if (safeThis == nullptr)
            return;

        auto file = chooser.getResult();
        if (file != juce::File())
        {
            auto presetName = makeSafePresetFileName (file.getFileNameWithoutExtension());
            safeThis->processorRef.savePresetToFile (file, presetName);
            safeThis->populateFactoryPresets();
        }

        safeThis->resetPresetSelection();
        safeThis->presetFileChooser.reset();
    });
}

void KillaBEditor::resetPresetSelection()
{
    presetCombo.setSelectedId (0, juce::dontSendNotification);
    presetCombo.setTextWhenNothingSelected ("Choose preset");
}

void KillaBEditor::timerCallback()
{
    constexpr float decayPerTick = 0.87f;
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

    wavePhase += 0.055f;
    orbitAngle += 0.009f;

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
    g.setColour (KBColours::gold.withAlpha (0.62f));
    g.drawRect (bounds, 1);
    g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 10.5f, juce::Font::bold)).withExtraKerningFactor (0.30f));
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
    auto innerBounds = bounds.reduced (12, 18);

    if (beeImage.isValid())
    {
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        auto target = innerBounds.toFloat();
        const auto imageBounds = beeImage.getBounds().toFloat();
        const float scale = juce::jmin (target.getWidth() / imageBounds.getWidth(),
                                        target.getHeight() / imageBounds.getHeight()) * 1.03f;
        const float drawW = imageBounds.getWidth() * scale;
        const float drawH = imageBounds.getHeight() * scale;
        const float drawX = target.getRight() - drawW - 4.0f;
        const float drawY = target.getBottom() - drawH - 2.0f;

        g.setOpacity (1.0f);
        g.drawImage (beeImage, juce::Rectangle<float> (drawX, drawY, drawW, drawH));
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
    paintDistortionMeters (g, distArea);
    paintEqGuides (g, eqArea);
    paintAdsrGuides (g, adsrArea);
    paintMascotPlaceholder (g, mascotArea);

    g.setColour (KBColours::gold.withAlpha (0.40f));
    g.drawVerticalLine (adsrArea.getX(), (float) bottomArea.getY() + 10.0f, (float) bottomArea.getBottom() - 10.0f);

    g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 11.0f, juce::Font::bold)).withExtraKerningFactor (0.18f));
    g.setColour (KBColours::textGold);
    g.drawText ("COMPRESSOR", compArea.getX() + 128, compArea.getY() + 104, 160, 14, juce::Justification::centred);
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

    auto bottom = bottomArea.reduced (16, 8);
    eqArea = bottom.removeFromLeft (540);
    adsrArea = bottom.removeFromLeft (190);
    lfoArea = bottom.removeFromLeft (170);
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
    const int masterKnobSize = 184;
    masterGainKnob.setBounds (orbArea.getCentreX() - masterKnobSize / 2,
                              orbArea.getCentreY() - masterKnobSize / 2 + 14,
                              masterKnobSize, masterKnobSize);
    masterGainLabel.setBounds (centerArea.getX() + 136, centerArea.getY() + 390, 120, 14);
    glideKnob.setBounds (centerArea.getX() + 22, centerArea.getBottom() - 180, 44, 44);
    glideLabel.setBounds (centerArea.getX() + 8, centerArea.getBottom() - 142, 72, 14);
    cutoffKnob.setBounds (centerArea.getX() + 22, centerArea.getBottom() - 124, 44, 44);
    cutoffLabel.setBounds (centerArea.getX() + 0, centerArea.getBottom() - 86, 88, 14);
    voicesLabel.setBounds (centerArea.getX() + 148, centerArea.getBottom() - 70, 120, 14);
    voicesBox.setBounds (centerArea.getX() + 120, centerArea.getBottom() - 42, 170, 24);

    driveKnob.setBounds (distArea.getX() + 30, distArea.getY() + 192, 72, 72);
    driveLabel.setBounds (distArea.getX() + 20, distArea.getY() + 268, 92, 14);

    const int compStartX = compArea.getX() + 38;
    const int compY = compArea.getBottom() - 78;
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
    masterGainKnob.toFront (true);

    const int meterY = eqArea.getY() + 36;
    const int meterH = 220;
    const int eqKnobSize = 124;
    const int eqKnobY = eqArea.getY() + 78;
    const int eqLabelY = eqKnobY + eqKnobSize - 4;
    const int eqColW = eqArea.getWidth() / 3;
    const int eqLowX = eqArea.getX() + eqColW * 0 + (eqColW - eqKnobSize) / 2 + 10;
    const int eqMidX = eqArea.getX() + eqColW * 1 + (eqColW - eqKnobSize) / 2 + 10;
    const int eqHighX = eqArea.getX() + eqColW * 2 + (eqColW - eqKnobSize) / 2 + 10;

    meterBarLow.setBounds (eqLowX - 30, meterY, 16, meterH);
    meterBarMid.setBounds (eqMidX - 30, meterY, 16, meterH);
    meterBarHigh.setBounds (eqHighX - 30, meterY, 16, meterH);
    eqLowKnob.setBounds (eqLowX, eqKnobY, eqKnobSize, eqKnobSize);
    eqLowLabel.setBounds (eqLowX + 20, eqLabelY, 84, 14);
    eqMidKnob.setBounds (eqMidX, eqKnobY, eqKnobSize, eqKnobSize);
    eqMidLabel.setBounds (eqMidX + 20, eqLabelY, 84, 14);
    eqHighKnob.setBounds (eqHighX, eqKnobY, eqKnobSize, eqKnobSize);
    eqHighLabel.setBounds (eqHighX + 20, eqLabelY, 84, 14);

    auto adsrBody = adsrArea.reduced (10, 42);
    const int sliderW = adsrBody.getWidth() / 4;
    attackSlider.setBounds (adsrBody.getX(), adsrBody.getY() + 28, sliderW, adsrBody.getHeight() - 34);
    decaySlider.setBounds (adsrBody.getX() + sliderW, adsrBody.getY() + 28, sliderW, adsrBody.getHeight() - 34);
    sustainSlider.setBounds (adsrBody.getX() + sliderW * 2, adsrBody.getY() + 28, sliderW, adsrBody.getHeight() - 34);
    releaseSlider.setBounds (adsrBody.getX() + sliderW * 3, adsrBody.getY() + 28, sliderW, adsrBody.getHeight() - 34);
    attackLabel.setBounds (adsrBody.getX(), adsrArea.getY() + 26, sliderW, 14);
    decayLabel.setBounds (adsrBody.getX() + sliderW, adsrArea.getY() + 26, sliderW, 14);
    sustainLabel.setBounds (adsrBody.getX() + sliderW * 2, adsrArea.getY() + 26, sliderW, 14);
    releaseLabel.setBounds (adsrBody.getX() + sliderW * 3, adsrArea.getY() + 26, sliderW, 14);

    lfoRoutingBox.setBounds (lfoArea.getX() + 12, lfoArea.getY() + 62, 110, 24);
    lfoRoutingLabel.setBounds (lfoArea.getX() + 12, lfoArea.getY() + 92, 110, 14);
    lfoTypeBox.setBounds (lfoArea.getX() + 12, lfoArea.getY() + 126, 110, 24);
    lfoTypeLabel.setBounds (lfoArea.getX() + 12, lfoArea.getY() + 156, 110, 14);
    lfoRateKnob.setBounds (lfoArea.getX() + 0, lfoArea.getBottom() - 98, 54, 54);
    lfoRateLabel.setBounds (lfoArea.getX() - 6, lfoArea.getBottom() - 40, 70, 14);
    lfoDepthKnob.setBounds (lfoArea.getX() + 58, lfoArea.getBottom() - 98, 54, 54);
    lfoDepthLabel.setBounds (lfoArea.getX() + 52, lfoArea.getBottom() - 40, 76, 14);

    auto keyboardBounds = keyboardArea.reduced (0, 5);
    keyboard.setBounds (keyboardBounds);
    keyboard.setKeyWidth ((float) keyboardBounds.getWidth() / (float) countWhiteKeysInRange (24, 108));

    osc1TypeBox.setBounds (0, 0, 0, 0);
    osc2TypeBox.setBounds (0, 0, 0, 0);
    osc1Label.setBounds (0, 0, 0, 0);
    osc2Label.setBounds (0, 0, 0, 0);
    oscMixKnob.setBounds (0, 0, 0, 0);
    oscMixLabel.setBounds (0, 0, 0, 0);
}
