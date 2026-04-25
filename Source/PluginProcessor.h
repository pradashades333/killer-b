#pragma once
#include <JuceHeader.h>
#include "SynthVoice.h"

// ---------------------------------------------------------------------------
// KillerBProcessor
//
// Signal chain (Phase 4 — UI Redesign)
// ──────────────────────────────────────
//   MIDI In
//     → juce::Synthesiser     (8 polyphonic voices)
//     → Drive                 (tanh soft-clip)
//     → Chorus                (juce::dsp::Chorus)
//     → Ping-Pong Delay       (2× juce::dsp::DelayLine, manual stereo cross-feed)
//     → Phaser                (juce::dsp::Phaser)
//     → Reverb                (juce::dsp::Reverb)
//     → 3-Band EQ             (Low Shelf / Mid Bell / High Shelf via IIR biquads)
//     → Compressor            (juce::dsp::Compressor)
//     → MasterGain            (juce::dsp::Gain)
//     → Band Metering         (3 mono IIR filters → peak RMS → std::atomic)
//     → Audio Out
//
// Thread-safety contract
// ──────────────────────
//   • All DSP objects are only ever touched on the audio thread.
//   • APVTS raw parameter pointers are std::atomic<float>* — safe to load()
//     from the audio thread at any time.
//   • levelLow / levelMid / levelHigh are std::atomic<float> written by the
//     audio thread (store, relaxed) and read by the GUI timer (load, relaxed).
//     Relaxed ordering is sufficient: we need atomic reads/writes to avoid UB,
//     but don't need cross-variable ordering guarantees.
// ---------------------------------------------------------------------------
class KillerBProcessor : public juce::AudioProcessor
{
public:
    KillerBProcessor();
    ~KillerBProcessor() override = default;

    // -----------------------------------------------------------------------
    // AudioProcessor overrides
    // -----------------------------------------------------------------------
    void prepareToPlay  (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi()  const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram  (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName  (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool savePresetToFile (const juce::File& file, const juce::String& presetName);
    bool loadPresetFromFile (const juce::File& file);
    static juce::File getUserPresetDirectory();
    static juce::File getBundledDrumDirectory();

    void setActiveDrumKit (int drumKitId);
    int getActiveDrumKit() const noexcept { return activeDrumKitId; }

    // -----------------------------------------------------------------------
    // Public — editor connects sliders / combo boxes and reads meter levels.
    // -----------------------------------------------------------------------
    juce::AudioProcessorValueTreeState apvts;

    // Piano keyboard state — written by processBlock (audio thread),
    // read by the MidiKeyboardComponent on the message thread.
    // MidiKeyboardState uses an internal CriticalSection for thread safety.
    juce::MidiKeyboardState keyboardState;

    // -----------------------------------------------------------------------
    // Band meter levels — written audio thread, read GUI timer (both relaxed).
    // Values are linear RMS (0.0 → ~0.7 for a 0 dBFS sine wave).
    // -----------------------------------------------------------------------
    std::atomic<float> levelLow  { 0.0f };
    std::atomic<float> levelMid  { 0.0f };
    std::atomic<float> levelHigh { 0.0f };

public:
    enum class DrumSampleCategory
    {
        bass808 = 0,
        kicks,
        snares,
        claps,
        hats,
        percs,
        voxFx,
        count
    };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    bool loadPresetXml (const juce::XmlElement& xml);

    static void applyDistortion    (juce::AudioBuffer<float>& buffer, float drive) noexcept;
    void        updateFXParameters () noexcept;
    void        updateFilterParameters() noexcept;
    void        updateEQParameters () noexcept;
    void        updateCompressorParameters() noexcept;

    // Compute per-band RMS from the final mixed buffer and store into atomics.
    // No allocations — uses processSample() on three persistent mono filters.
    void updateMeters (const juce::AudioBuffer<float>& buffer) noexcept;
    void refreshDrumKitFileCache();
    void loadDrumKitIntoSampler (int drumKitId);

    // -----------------------------------------------------------------------
    // Synth
    // -----------------------------------------------------------------------
    juce::Synthesiser synth;
    juce::Synthesiser drumSynth;
    juce::AudioFormatManager formatManager;
    static constexpr int NUM_VOICES = 8;
    static constexpr int NUM_DRUM_VOICES = 32;
    static constexpr int NUM_DRUM_KITS = 7;
    int activeDrumKitId = -1;
    std::array<juce::Array<juce::File>, static_cast<size_t> (DrumSampleCategory::count)> drumKitFiles;

    // -----------------------------------------------------------------------
    // FX Chain
    // -----------------------------------------------------------------------
    juce::dsp::Chorus<float>  chorus;
    juce::dsp::Phaser<float>  phaser;
    juce::dsp::Reverb         reverb;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>
        delayL { 192000 }, delayR { 192000 };

    // -----------------------------------------------------------------------
    // 3-Band EQ
    // ProcessorDuplicator wraps a mono IIR::Filter and duplicates it for every
    // channel in the ProcessSpec — the only JUCE-idiomatic way to run a
    // stereo biquad from a single shared Coefficients object.
    // -----------------------------------------------------------------------
    using StereoIIR = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                       juce::dsp::IIR::Coefficients<float>>;
    StereoIIR filterStage1, filterStage2;
    StereoIIR eqLow, eqMid, eqHigh;

    // -----------------------------------------------------------------------
    // Compressor + Master Gain (post-EQ)
    // -----------------------------------------------------------------------
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Gain<float>       masterGainDSP;

    // -----------------------------------------------------------------------
    // Band metering filters (mono, fixed coefficients set in prepareToPlay)
    // Each one is an independent biquad operating on the L+R mono mix.
    //   meterLow  — LPF  at  250 Hz
    //   meterMid  — BPF  at  800 Hz (Q = 0.7 for a wide mid band)
    //   meterHigh — HPF  at 2500 Hz
    // -----------------------------------------------------------------------
    juce::dsp::IIR::Filter<float> meterLow, meterMid, meterHigh;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KillerBProcessor)
};
