#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr int noDrumKitSelected = -1;
    constexpr int numDrumKits = 7;
    constexpr double drumTailOffSeconds = 0.001;
    constexpr double drumMaxSampleLengthSeconds = 30.0;

    struct DrumNoteAssignment
    {
        int midiNote;
        KillerBProcessor::DrumSampleCategory category;
        int sampleOffset;
    };

    struct DrumKitDefinition
    {
        std::array<int, static_cast<size_t> (KillerBProcessor::DrumSampleCategory::count)> categoryOffsets;
    };

    bool nameContainsAny (const juce::String& name, std::initializer_list<const char*> needles)
    {
        for (const auto* needle : needles)
            if (name.containsIgnoreCase (needle))
                return true;

        return false;
    }

    KillerBProcessor::DrumSampleCategory categoriseDrumSample (const juce::String& fileName)
    {
        if (nameContainsAny (fileName, { "808" }))
            return KillerBProcessor::DrumSampleCategory::bass808;

        if (nameContainsAny (fileName, { "kick" }))
            return KillerBProcessor::DrumSampleCategory::kicks;

        if (nameContainsAny (fileName, { "snare" }))
            return KillerBProcessor::DrumSampleCategory::snares;

        if (nameContainsAny (fileName, { "clap" }))
            return KillerBProcessor::DrumSampleCategory::claps;

        if (nameContainsAny (fileName, { "hat", "(oh)", "openh", "ophat", "cym", "crash" }))
            return KillerBProcessor::DrumSampleCategory::hats;

        if (nameContainsAny (fileName, { "perc", "rim", "tom", "cowbell", "bell" }))
            return KillerBProcessor::DrumSampleCategory::percs;

        return KillerBProcessor::DrumSampleCategory::voxFx;
    }

    constexpr std::array<DrumNoteAssignment, 25> drumNoteAssignments
    {{
        { 48, KillerBProcessor::DrumSampleCategory::kicks, 0 },   // C3
        { 49, KillerBProcessor::DrumSampleCategory::bass808, 0 }, // C#3
        { 50, KillerBProcessor::DrumSampleCategory::snares, 0 },  // D3
        { 51, KillerBProcessor::DrumSampleCategory::claps, 0 },   // D#3
        { 52, KillerBProcessor::DrumSampleCategory::percs, 0 },   // E3
        { 53, KillerBProcessor::DrumSampleCategory::hats, 0 },    // F3
        { 54, KillerBProcessor::DrumSampleCategory::hats, 1 },    // F#3
        { 55, KillerBProcessor::DrumSampleCategory::bass808, 1 }, // G3
        { 56, KillerBProcessor::DrumSampleCategory::percs, 1 },   // G#3
        { 57, KillerBProcessor::DrumSampleCategory::voxFx, 0 },   // A3
        { 58, KillerBProcessor::DrumSampleCategory::voxFx, 1 },   // A#3
        { 59, KillerBProcessor::DrumSampleCategory::snares, 1 },  // B3
        { 60, KillerBProcessor::DrumSampleCategory::claps, 1 },   // C4
        { 61, KillerBProcessor::DrumSampleCategory::bass808, 2 }, // C#4
        { 62, KillerBProcessor::DrumSampleCategory::snares, 2 },  // D4
        { 63, KillerBProcessor::DrumSampleCategory::claps, 2 },   // D#4
        { 64, KillerBProcessor::DrumSampleCategory::percs, 2 },   // E4
        { 65, KillerBProcessor::DrumSampleCategory::hats, 2 },    // F4
        { 66, KillerBProcessor::DrumSampleCategory::hats, 3 },    // F#4
        { 67, KillerBProcessor::DrumSampleCategory::kicks, 1 },   // G4
        { 68, KillerBProcessor::DrumSampleCategory::percs, 3 },   // G#4
        { 69, KillerBProcessor::DrumSampleCategory::claps, 3 },   // A4
        { 70, KillerBProcessor::DrumSampleCategory::voxFx, 2 },   // A#4
        { 71, KillerBProcessor::DrumSampleCategory::snares, 3 },  // B4
        { 72, KillerBProcessor::DrumSampleCategory::hats, 4 }     // C5
    }};

    constexpr std::array<DrumKitDefinition, numDrumKits> drumKitDefinitions
    {{
        { { 0, 0, 0, 0, 0, 0, 0 } },
        { { 2, 1, 2, 2, 3, 1, 1 } },
        { { 4, 3, 4, 4, 6, 3, 3 } },
        { { 6, 5, 6, 6, 9, 5, 5 } },
        { { 8, 7, 8, 8, 12, 7, 7 } },
        { { 10, 9, 10, 10, 15, 9, 9 } },
        { { 12, 11, 12, 12, 18, 11, 11 } }
    }};
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
KillerBProcessor::KillerBProcessor()
    : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "KillerBState", createParameterLayout())
{
    formatManager.registerBasicFormats();

    for (int i = 0; i < NUM_VOICES; ++i)
    {
        auto* voice = new SynthVoice();
        voice->setParameterPointers (
            apvts.getRawParameterValue ("osc1type"),
            apvts.getRawParameterValue ("osc2type"),
            apvts.getRawParameterValue ("oscmix"),
            apvts.getRawParameterValue ("attack"),
            apvts.getRawParameterValue ("decay"),
            apvts.getRawParameterValue ("sustain"),
            apvts.getRawParameterValue ("release"),
            apvts.getRawParameterValue ("glide")
        );
        synth.addVoice (voice);
    }
    synth.addSound (new SynthSound());

    for (int i = 0; i < NUM_DRUM_VOICES; ++i)
        drumSynth.addVoice (new juce::SamplerVoice());

    refreshDrumKitFileCache();
}

juce::File KillerBProcessor::getBundledDrumDirectory()
{
    return juce::File ("C:/Files/Coding/killer b/Extras/wetransfer_turnmeupb12_-projectdata-zip_2026-03-27_2303/Turnmeupb12_[ProjectData]/Turnmeupb12_[ProjectData]");
}

void KillerBProcessor::refreshDrumKitFileCache()
{
    for (auto& files : drumKitFiles)
        files.clear();

    const auto drumDirectory = getBundledDrumDirectory();
    if (! drumDirectory.isDirectory())
        return;

    juce::Array<juce::File> files;
    drumDirectory.findChildFiles (files, juce::File::findFiles, false, "*.wav");
    drumDirectory.findChildFiles (files, juce::File::findFiles, false, "*.WAV");

    struct FileNameComparator
    {
        static int compareElements (const juce::File& a, const juce::File& b)
        {
            return a.getFileName().compareIgnoreCase (b.getFileName());
        }
    };

    FileNameComparator comparator;
    files.sort (comparator);

    for (const auto& file : files)
    {
        const auto category = categoriseDrumSample (file.getFileNameWithoutExtension());
        drumKitFiles[static_cast<size_t> (category)].addIfNotAlreadyThere (file);
    }
}

void KillerBProcessor::loadDrumKitIntoSampler (int drumKitId)
{
    drumSynth.clearSounds();

    if (drumKitId < 0 || drumKitId >= NUM_DRUM_KITS)
        return;

    const auto& kit = drumKitDefinitions[(size_t) drumKitId];

    for (const auto& assignment : drumNoteAssignments)
    {
        const auto categoryIndex = static_cast<size_t> (assignment.category);
        const auto& files = drumKitFiles[categoryIndex];
        if (files.isEmpty())
            continue;

        const int categoryBaseOffset = kit.categoryOffsets[categoryIndex];
        const int sampleIndex = (categoryBaseOffset + assignment.sampleOffset) % files.size();
        const auto& file = files.getReference (sampleIndex);
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));

        if (reader == nullptr)
            continue;

        juce::BigInteger noteRange;
        const int midiNote = assignment.midiNote;
        noteRange.setBit (midiNote);

        drumSynth.addSound (new juce::SamplerSound (file.getFileNameWithoutExtension(),
                                                    *reader,
                                                    noteRange,
                                                    midiNote,
                                                    0.0,
                                                    drumTailOffSeconds,
                                                    drumMaxSampleLengthSeconds));
    }
}

void KillerBProcessor::setActiveDrumKit (int drumKitId)
{
    if (drumKitId == activeDrumKitId)
        return;

    suspendProcessing (true);
    {
        const juce::ScopedLock sl (getCallbackLock());
        activeDrumKitId = drumKitId;
        loadDrumKitIntoSampler (drumKitId);
    }
    suspendProcessing (false);
}

// ---------------------------------------------------------------------------
// Parameter layout
// ---------------------------------------------------------------------------
juce::AudioProcessorValueTreeState::ParameterLayout
KillerBProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // --- Oscillator types ---------------------------------------------------
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        "osc1type", "OSC1 Type", juce::StringArray { "Sine", "Square", "Saw" }, 2));
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        "osc2type", "OSC2 Type", juce::StringArray { "Sine", "Square", "Saw" }, 2));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "oscmix", "OSC Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    // --- ADSR ---------------------------------------------------------------
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "attack",  "Attack",
        juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.35f), 0.005f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "decay",   "Decay",
        juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.35f), 0.3f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "sustain", "Sustain",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.7f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "release", "Release",
        juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.35f), 0.5f));

    // --- Glide & Drive ------------------------------------------------------
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "glide", "Glide",
        juce::NormalisableRange<float> (0.0f, 2.0f, 0.001f, 0.5f), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "drive", "Drive",
        juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f, 0.4f), 1.0f));

    // --- Filter -------------------------------------------------------------
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        "filterType", "Filter Type", juce::StringArray { "None", "Low-pass", "High-pass" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        "filterSlope", "Filter Slope", juce::StringArray { "12 dB/oct", "24 dB/oct" }, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "filterCutoff", "Filter Cutoff",
        juce::NormalisableRange<float> (40.0f, 16000.0f, 1.0f, 0.35f), 12000.0f));

    // --- FX Rack ------------------------------------------------------------
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "CHORUS_AMOUNT", "Chorus Rate",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.3f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "CHORUS_MIX", "Chorus Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "DELAY_AMOUNT", "Delay Time",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f, 0.5f), 0.375f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "DELAY_MIX", "Delay Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "PHASER_AMOUNT", "Phaser Freq",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.2f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "PHASER_MIX", "Phaser Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "REVERB_AMOUNT", "Reverb Size",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "REVERB_MIX", "Reverb Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    // -----------------------------------------------------------------------
    // 3-Band EQ — gain in dB, default 0 (unity, transparent)
    // -----------------------------------------------------------------------
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "LOW_GAIN",  "Low Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "MID_GAIN",  "Mid Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "HIGH_GAIN", "High Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f));

    // -----------------------------------------------------------------------
    // Master Gain & Compressor (new in Phase 4)
    // -----------------------------------------------------------------------
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "MASTER_GAIN", "Master Gain",
        juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "COMP_THRESHOLD", "Comp Threshold",
        juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -12.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "COMP_RATIO", "Comp Ratio",
        juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f, 0.4f), 4.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "COMP_ATTACK", "Comp Attack",
        juce::NormalisableRange<float> (0.1f, 200.0f, 0.1f, 0.4f), 10.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "COMP_RELEASE", "Comp Release",
        juce::NormalisableRange<float> (10.0f, 1000.0f, 1.0f, 0.4f), 100.0f));

    return layout;
}

// ---------------------------------------------------------------------------
// Playback lifecycle
// ---------------------------------------------------------------------------
void KillerBProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // --- Synth voices -------------------------------------------------------
    synth.setCurrentPlaybackSampleRate (sampleRate);
    drumSynth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            v->prepareToPlay (sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    // --- Shared specs -------------------------------------------------------
    juce::dsp::ProcessSpec stereoSpec;
    stereoSpec.sampleRate       = sampleRate;
    stereoSpec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    stereoSpec.numChannels      = 2;

    juce::dsp::ProcessSpec monoSpec = stereoSpec;
    monoSpec.numChannels = 1;

    // --- FX chain -----------------------------------------------------------
    chorus.prepare (stereoSpec);   chorus.reset();
    phaser.prepare (stereoSpec);   phaser.reset();
    reverb.prepare (stereoSpec);   reverb.reset();
    delayL.prepare (monoSpec);     delayL.reset();
    delayR.prepare (monoSpec);     delayR.reset();

    filterStage1.prepare (stereoSpec); filterStage1.reset();
    filterStage2.prepare (stereoSpec); filterStage2.reset();

    // --- 3-Band EQ ----------------------------------------------------------
    eqLow.prepare  (stereoSpec);   eqLow.reset();
    eqMid.prepare  (stereoSpec);   eqMid.reset();
    eqHigh.prepare (stereoSpec);   eqHigh.reset();

    // --- Compressor & Master Gain -------------------------------------------
    compressor.prepare   (stereoSpec);  compressor.reset();
    masterGainDSP.prepare (stereoSpec); masterGainDSP.reset();

    // --- Band metering filters (mono, fixed frequency, no APVTS binding) ---
    meterLow.prepare  (monoSpec);  meterLow.reset();
    meterMid.prepare  (monoSpec);  meterMid.reset();
    meterHigh.prepare (monoSpec);  meterHigh.reset();

    // Set fixed crossover coefficients.  These never change at runtime.
    *meterLow.coefficients  = *juce::dsp::IIR::Coefficients<float>::makeLowPass  (sampleRate,  250.0f);
    *meterMid.coefficients  = *juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate,  800.0f, 0.7f);
    *meterHigh.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 2500.0f);

    // Push initial EQ & FX parameter values so the first callback is clean.
    updateFilterParameters();
    updateFXParameters();
    updateEQParameters();
    updateCompressorParameters();
}

void KillerBProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KillerBProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;

    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}
#endif

// ---------------------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------------------
void KillerBProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Feed MIDI through keyboard state (enables on-screen piano to reflect notes)
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    // 1 — Synth voices
    if (activeDrumKitId >= 0)
        drumSynth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());
    else
        synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());

    // 2 — Drive
    const float drive = apvts.getRawParameterValue ("drive")->load();
    if (drive > 1.05f)
        applyDistortion (buffer, drive);

    // 3 — Update FX parameters (reflects host automation immediately)
    auto block   = juce::dsp::AudioBlock<float> (buffer);
    auto context = juce::dsp::ProcessContextReplacing<float> (block);

    updateFilterParameters();
    const int filterType = (int) apvts.getRawParameterValue ("filterType")->load();
    const int filterSlope = (int) apvts.getRawParameterValue ("filterSlope")->load();

    if (filterType != 0)
    {
        filterStage1.process (context);

        if (filterSlope == 1)
            filterStage2.process (context);
    }

    updateFXParameters();

    // 4 — Chorus
    chorus.process (context);

    // 5 — Ping-pong delay
    {
        const float delayAmt     = apvts.getRawParameterValue ("DELAY_AMOUNT")->load();
        const float delayMix     = apvts.getRawParameterValue ("DELAY_MIX")->load();
        const float delaySamples = juce::jmax (1.0f, delayAmt * (float) getSampleRate());
        constexpr float feedback = 0.45f;

        float* dataL = buffer.getWritePointer (0);
        float* dataR = buffer.getWritePointer (1);

        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            const float inL  = dataL[s];
            const float inR  = dataR[s];
            const float wetL = delayL.popSample (0, delaySamples, true);
            const float wetR = delayR.popSample (0, delaySamples, true);
            delayL.pushSample (0, inL + wetR * feedback);
            delayR.pushSample (0, inR + wetL * feedback);
            dataL[s] = inL + (wetL - inL) * delayMix;
            dataR[s] = inR + (wetR - inR) * delayMix;
        }
    }

    // 6 — Phaser
    phaser.process (context);

    // 7 — Reverb
    reverb.process (context);

    // 8 — 3-Band EQ  (three serial biquad stages on the stereo signal)
    updateEQParameters();
    eqLow.process  (context);
    eqMid.process  (context);
    eqHigh.process (context);

    // 9 — Compressor
    updateCompressorParameters();
    compressor.process (context);

    // 10 — Master Gain
    masterGainDSP.setGainDecibels (apvts.getRawParameterValue ("MASTER_GAIN")->load());
    masterGainDSP.process (context);

    // 11 — Band metering  (reads the final processed output → atomics → GUI)
    updateMeters (buffer);
}

// ---------------------------------------------------------------------------
// updateFXParameters
// ---------------------------------------------------------------------------
void KillerBProcessor::updateFXParameters() noexcept
{
    const float chorusAmt = apvts.getRawParameterValue ("CHORUS_AMOUNT")->load();
    const float chorusMix = apvts.getRawParameterValue ("CHORUS_MIX")->load();
    chorus.setRate        (0.1f + chorusAmt * 4.9f);
    chorus.setDepth       (0.5f);
    chorus.setCentreDelay (7.0f);
    chorus.setFeedback    (0.0f);
    chorus.setMix         (chorusMix);

    const float phaserAmt = apvts.getRawParameterValue ("PHASER_AMOUNT")->load();
    const float phaserMix = apvts.getRawParameterValue ("PHASER_MIX")->load();
    phaser.setCentreFrequency (100.0f + phaserAmt * 7900.0f);
    phaser.setRate            (0.5f);
    phaser.setDepth           (0.7f);
    phaser.setFeedback        (0.7f);
    phaser.setMix             (phaserMix);

    const float reverbAmt = apvts.getRawParameterValue ("REVERB_AMOUNT")->load();
    const float reverbMix = apvts.getRawParameterValue ("REVERB_MIX")->load();
    juce::dsp::Reverb::Parameters revP;
    revP.roomSize   = reverbAmt;
    revP.damping    = 0.5f;
    revP.wetLevel   = reverbMix;
    revP.dryLevel   = 1.0f - reverbMix;
    revP.width      = 1.0f;
    revP.freezeMode = 0.0f;
    reverb.setParameters (revP);
}

// ---------------------------------------------------------------------------
// updateFilterParameters
// ---------------------------------------------------------------------------
void KillerBProcessor::updateFilterParameters() noexcept
{
    const double sr = getSampleRate();
    const int filterType = (int) apvts.getRawParameterValue ("filterType")->load();
    const float cutoff = juce::jlimit (40.0f, 16000.0f,
                                      apvts.getRawParameterValue ("filterCutoff")->load());

    juce::dsp::IIR::Coefficients<float>::Ptr coefficients;

    if (filterType == 2)
        coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, cutoff, 0.707f);
    else
        coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sr, cutoff, 0.707f);

    *filterStage1.state = *coefficients;
    *filterStage2.state = *coefficients;
}

// ---------------------------------------------------------------------------
// updateEQParameters
// ---------------------------------------------------------------------------
void KillerBProcessor::updateEQParameters() noexcept
{
    const double sr = getSampleRate();

    const float lowGain  = juce::Decibels::decibelsToGain (
        apvts.getRawParameterValue ("LOW_GAIN")->load());
    const float midGain  = juce::Decibels::decibelsToGain (
        apvts.getRawParameterValue ("MID_GAIN")->load());
    const float highGain = juce::Decibels::decibelsToGain (
        apvts.getRawParameterValue ("HIGH_GAIN")->load());

    *eqLow.state  = *juce::dsp::IIR::Coefficients<float>::makeLowShelf  (sr, 250.0f,  0.707f, lowGain);
    *eqMid.state  = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (sr, 800.0f,  0.8f,   midGain);
    *eqHigh.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (sr, 2500.0f, 0.707f, highGain);
}

// ---------------------------------------------------------------------------
// updateCompressorParameters
// ---------------------------------------------------------------------------
void KillerBProcessor::updateCompressorParameters() noexcept
{
    compressor.setThreshold (apvts.getRawParameterValue ("COMP_THRESHOLD")->load());
    compressor.setRatio     (apvts.getRawParameterValue ("COMP_RATIO")->load());
    compressor.setAttack    (apvts.getRawParameterValue ("COMP_ATTACK")->load());
    compressor.setRelease   (apvts.getRawParameterValue ("COMP_RELEASE")->load());
}

// ---------------------------------------------------------------------------
// updateMeters
// ---------------------------------------------------------------------------
void KillerBProcessor::updateMeters (const juce::AudioBuffer<float>& buffer) noexcept
{
    const int   N   = buffer.getNumSamples();
    if (N == 0) return;

    const float* chL = buffer.getReadPointer (0);
    const float* chR = buffer.getReadPointer (1);

    float sumLow = 0.0f, sumMid = 0.0f, sumHigh = 0.0f;

    for (int s = 0; s < N; ++s)
    {
        const float mono = (chL[s] + chR[s]) * 0.5f;

        const float fLow  = meterLow.processSample  (mono);
        const float fMid  = meterMid.processSample  (mono);
        const float fHigh = meterHigh.processSample (mono);

        sumLow  += fLow  * fLow;
        sumMid  += fMid  * fMid;
        sumHigh += fHigh * fHigh;
    }

    const float inv = 1.0f / (float) N;
    levelLow.store  (std::sqrt (sumLow  * inv), std::memory_order_relaxed);
    levelMid.store  (std::sqrt (sumMid  * inv), std::memory_order_relaxed);
    levelHigh.store (std::sqrt (sumHigh * inv), std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// Editor factory
// ---------------------------------------------------------------------------
juce::AudioProcessorEditor* KillerBProcessor::createEditor()
{
    return new KillaBEditor (*this);
}

// ---------------------------------------------------------------------------
// Drive
// ---------------------------------------------------------------------------
void KillerBProcessor::applyDistortion (juce::AudioBuffer<float>& buffer,
                                         float drive) noexcept
{
    const float normFactor = 1.0f / std::tanh (drive);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer (ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
            data[s] = std::tanh (data[s] * drive) * normFactor;
    }
}

// ---------------------------------------------------------------------------
// Preset save / load
// ---------------------------------------------------------------------------
void KillerBProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("drumKitId", activeDrumKitId, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void KillerBProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        loadPresetXml (*xml);
}

bool KillerBProcessor::savePresetToFile (const juce::File& file, const juce::String& presetName)
{
    auto target = file;
    if (! target.hasFileExtension (".kbpreset"))
        target = target.withFileExtension (".kbpreset");

    if (auto parent = target.getParentDirectory(); parent != juce::File())
        parent.createDirectory();

    auto state = apvts.copyState();
    state.setProperty ("presetName", presetName, nullptr);
    state.setProperty ("drumKitId", activeDrumKitId, nullptr);

    if (auto xml = state.createXml())
        return xml->writeTo (target);

    return false;
}

bool KillerBProcessor::loadPresetFromFile (const juce::File& file)
{
    if (! file.existsAsFile())
        return false;

    if (auto xml = juce::XmlDocument::parse (file))
        return loadPresetXml (*xml);

    return false;
}

juce::File KillerBProcessor::getUserPresetDirectory()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("Killer B Synth Presets");
}

bool KillerBProcessor::loadPresetXml (const juce::XmlElement& xml)
{
    if (! xml.hasTagName (apvts.state.getType()))
        return false;

    auto state = juce::ValueTree::fromXml (xml);
    if (! state.isValid())
        return false;

    const int drumKitId = (int) state.getProperty ("drumKitId", noDrumKitSelected);
    apvts.replaceState (state);
    setActiveDrumKit (drumKitId);
    return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KillerBProcessor();
}
