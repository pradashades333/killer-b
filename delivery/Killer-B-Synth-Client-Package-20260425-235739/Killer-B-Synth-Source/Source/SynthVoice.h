#pragma once
#include <JuceHeader.h>

// ---------------------------------------------------------------------------
// SynthSound — a trivial sound that applies to all MIDI notes/channels.
// ---------------------------------------------------------------------------
struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote    (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

// ---------------------------------------------------------------------------
// SynthVoice
//
// Architecture overview
// ─────────────────────
//   • Two independent oscillators (OSC1 / OSC2) each with Sine / Square / Saw.
//   • OSC2 is given a tiny static detune (+0.3 cents) to add natural width.
//   • Per-voice ADSR (juce::ADSR).
//   • Per-voice portamento via juce::SmoothedValue<double> on the frequency.
//     Glide activates when a voice is stolen / re-triggered while still active.
//   • All musically interesting parameters are read in real-time from raw
//     APVTS pointers — no copy cost, always in sync with the host automation.
// ---------------------------------------------------------------------------
class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice() = default;

    // Call once from PluginProcessor::prepareToPlay.
    void prepareToPlay (double sampleRate, int /*samplesPerBlock*/, int /*numChannels*/)
    {
        adsr.setSampleRate (sampleRate);
        smoothedFreq.reset (sampleRate, 0.0);  // ramp time set per note-on
        isPrepared = true;
    }

    // Wire up shared APVTS raw pointers so every voice reads the same params.
    void setParameterPointers (std::atomic<float>* osc1Type_,
                                std::atomic<float>* osc2Type_,
                                std::atomic<float>* oscMix_,
                                std::atomic<float>* attack_,
                                std::atomic<float>* decay_,
                                std::atomic<float>* sustain_,
                                std::atomic<float>* release_,
                                std::atomic<float>* glide_)
    {
        osc1TypeParam  = osc1Type_;
        osc2TypeParam  = osc2Type_;
        oscMixParam    = oscMix_;
        attackParam    = attack_;
        decayParam     = decay_;
        sustainParam   = sustain_;
        releaseParam   = release_;
        glideParam     = glide_;
    }

    // -----------------------------------------------------------------------
    // juce::SynthesiserVoice overrides
    // -----------------------------------------------------------------------
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int /*pitchWheel*/) override
    {
        jassert (isPrepared);

        const double targetFreq = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        const float  glideTime  = glideParam ? glideParam->load() : 0.0f;

        // Portamento: slide from current frequency only when the voice was
        // already sounding and the user has dialled in some glide time.
        if (glideTime > 0.001f && voiceWasActive)
        {
            smoothedFreq.reset (getSampleRate(), static_cast<double> (glideTime));
        }
        else
        {
            // Hard retrigger — reset phases for a clean transient.
            smoothedFreq.reset (getSampleRate(), 0.0);
            phase1 = 0.0;
            phase2 = 0.0;
        }

        smoothedFreq.setTargetValue (targetFreq);
        voiceWasActive = true;
        velocity_      = velocity;

        updateADSR();
        adsr.noteOn();
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            adsr.noteOff();
        }
        else
        {
            adsr.reset();
            voiceWasActive = false;
            clearCurrentNote();
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample, int numSamples) override
    {
        if (! isVoiceActive()) return;

        // Re-read ADSR params every block so host automation is reflected
        // immediately without the voice needing to be re-triggered.
        updateADSR();

        const int   osc1T = osc1TypeParam ? static_cast<int> (osc1TypeParam->load()) : 2;
        const int   osc2T = osc2TypeParam ? static_cast<int> (osc2TypeParam->load()) : 2;
        const float mix   = oscMixParam   ? oscMixParam->load() : 0.5f;

        const double sampleRate = getSampleRate();

        for (int s = 0; s < numSamples; ++s)
        {
            const double freq = smoothedFreq.getNextValue();

            // OSC 1 — fundamental pitch
            const double inc1 = freq / sampleRate;
            const float  osc1 = generateSample (osc1T, phase1);
            phase1 = std::fmod (phase1 + inc1, 1.0);

            // OSC 2 — tiny detune (+0.3 cents) for natural thickness
            const double inc2 = (freq * 1.001731) / sampleRate;  // ~3 cents up
            const float  osc2 = generateSample (osc2T, phase2);
            phase2 = std::fmod (phase2 + inc2, 1.0);

            const float mixed    = osc1 * (1.0f - mix) + osc2 * mix;
            const float envelope = adsr.getNextSample();
            const float out      = mixed * envelope * velocity_ * 0.4f; // headroom

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample (ch, startSample + s, out);
        }

        if (! adsr.isActive())
        {
            voiceWasActive = false;
            clearCurrentNote();
        }
    }

private:
    // -----------------------------------------------------------------------
    // Waveform generators  (phase range: [0, 1))
    // -----------------------------------------------------------------------
    static float generateSample (int oscType, double phase) noexcept
    {
        switch (oscType)
        {
            case 0:  // Sine — smooth sub bass / pads
                return static_cast<float> (std::sin (phase * juce::MathConstants<double>::twoPi));

            case 1:  // Square — aggressive plucks / leads
                return phase < 0.5 ? 1.0f : -1.0f;

            case 2:  // Saw — the 808 workhorse
            default:
                return static_cast<float> (2.0 * phase - 1.0);
        }
    }

    // Sync ADSR parameters from APVTS pointers (called every block).
    void updateADSR()
    {
        juce::ADSR::Parameters p;
        p.attack  = attackParam  ? juce::jmax (0.001f, attackParam->load())  : 0.01f;
        p.decay   = decayParam   ? juce::jmax (0.001f, decayParam->load())   : 0.3f;
        p.sustain = sustainParam ? juce::jlimit (0.0f, 1.0f, sustainParam->load()) : 0.7f;
        p.release = releaseParam ? juce::jmax (0.001f, releaseParam->load()) : 0.5f;
        adsr.setParameters (p);
    }

    // -----------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------
    juce::ADSR                  adsr;
    juce::SmoothedValue<double> smoothedFreq { 440.0 };

    double phase1        = 0.0;
    double phase2        = 0.0;
    float  velocity_     = 1.0f;
    bool   isPrepared    = false;
    bool   voiceWasActive = false;

    // Raw APVTS pointers — read-only, owned by AudioProcessorValueTreeState.
    std::atomic<float>* osc1TypeParam  = nullptr;
    std::atomic<float>* osc2TypeParam  = nullptr;
    std::atomic<float>* oscMixParam    = nullptr;
    std::atomic<float>* attackParam    = nullptr;
    std::atomic<float>* decayParam     = nullptr;
    std::atomic<float>* sustainParam   = nullptr;
    std::atomic<float>* releaseParam   = nullptr;
    std::atomic<float>* glideParam     = nullptr;
};
