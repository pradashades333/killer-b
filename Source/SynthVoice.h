#pragma once
#include <JuceHeader.h>

struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice() = default;

    void prepareToPlay (double sampleRate, int /*samplesPerBlock*/, int /*numChannels*/)
    {
        adsr.setSampleRate (sampleRate);
        smoothedFreq.reset (sampleRate, 0.0);
        isPrepared = true;
    }

    void setParameterPointers (std::atomic<float>* osc1Type_,
                               std::atomic<float>* osc2Type_,
                               std::atomic<float>* oscMix_,
                               std::atomic<float>* attack_,
                               std::atomic<float>* decay_,
                               std::atomic<float>* sustain_,
                               std::atomic<float>* release_,
                               std::atomic<float>* glide_,
                               std::atomic<float>* lfoRate_,
                               std::atomic<float>* lfoDepth_,
                               std::atomic<float>* lfoRouting_,
                               std::atomic<float>* lfoType_)
    {
        osc1TypeParam = osc1Type_;
        osc2TypeParam = osc2Type_;
        oscMixParam = oscMix_;
        attackParam = attack_;
        decayParam = decay_;
        sustainParam = sustain_;
        releaseParam = release_;
        glideParam = glide_;
        lfoRateParam = lfoRate_;
        lfoDepthParam = lfoDepth_;
        lfoRoutingParam = lfoRouting_;
        lfoTypeParam = lfoType_;
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int /*pitchWheel*/) override
    {
        jassert (isPrepared);

        const double targetFreq = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        const float glideTime = glideParam ? glideParam->load() : 0.0f;

        if (glideTime > 0.001f && voiceWasActive)
        {
            smoothedFreq.reset (getSampleRate(), static_cast<double> (glideTime));
        }
        else
        {
            smoothedFreq.reset (getSampleRate(), 0.0);
            phase1 = 0.0;
            phase2 = 0.0;
            lfoPhase = 0.0;
        }

        smoothedFreq.setTargetValue (targetFreq);
        voiceWasActive = true;
        velocity_ = velocity;
        lastEnvelope = 0.0f;

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
        if (! isVoiceActive())
            return;

        updateADSR();

        const int osc1T = osc1TypeParam ? static_cast<int> (osc1TypeParam->load()) : 2;
        const int osc2T = osc2TypeParam ? static_cast<int> (osc2TypeParam->load()) : 2;
        const float mix = oscMixParam ? oscMixParam->load() : 0.5f;
        const int lfoRouting = lfoRoutingParam ? static_cast<int> (lfoRoutingParam->load()) : 0;
        const float lfoRateHz = 0.05f + (lfoRateParam ? lfoRateParam->load() : 0.0f) * 11.95f;
        const float lfoDepth = lfoDepthParam ? lfoDepthParam->load() : 0.0f;

        const double sampleRate = getSampleRate();

        for (int s = 0; s < numSamples; ++s)
        {
            const float lfoValue = getLfoValue();
            const float pitchModSemitones = lfoRouting == 1 ? lfoValue * lfoDepth * 1.75f : 0.0f;
            const double freq = smoothedFreq.getNextValue()
                              * std::pow (2.0, static_cast<double> (pitchModSemitones) / 12.0);

            const double inc1 = freq / sampleRate;
            const float osc1 = generateSample (osc1T, phase1);
            phase1 = std::fmod (phase1 + inc1, 1.0);

            const double inc2 = (freq * 1.001731) / sampleRate;
            const float osc2 = generateSample (osc2T, phase2);
            phase2 = std::fmod (phase2 + inc2, 1.0);

            const float envelope = adsr.getNextSample();
            const float transient = computeTransient (osc1T, osc2T, mix, envelope, lastEnvelope);
            const float mixed = osc1 * (1.0f - mix) + osc2 * mix + transient;
            const float out = mixed * envelope * velocity_ * 0.35f;

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample (ch, startSample + s, out);

            lfoPhase = std::fmod (lfoPhase + static_cast<double> (lfoRateHz) / sampleRate, 1.0);
            lastEnvelope = envelope;
        }

        if (! adsr.isActive())
        {
            voiceWasActive = false;
            clearCurrentNote();
        }
    }

private:
    static float generateSample (int oscType, double phase) noexcept
    {
        switch (oscType)
        {
            case 0:
            {
                const auto radians = phase * juce::MathConstants<double>::twoPi;
                return static_cast<float> (std::sin (radians)
                                         + 0.12 * std::sin (radians * 2.0)
                                         + 0.04 * std::sin (radians * 3.0));
            }

            case 1:
            {
                constexpr double pulseWidth = 0.38;
                const float pulse = phase < pulseWidth ? 1.0f : -1.0f;
                const float sub = static_cast<float> ((phase < 0.5 ? 1.0 : -1.0) * 0.18);
                return pulse + sub;
            }

            case 2:
            default:
            {
                const float saw = static_cast<float> (2.0 * phase - 1.0);
                const float octave = static_cast<float> (2.0 * std::fmod (phase * 2.0, 1.0) - 1.0) * 0.22f;
                return saw + octave;
            }
        }
    }

    float getLfoValue() const noexcept
    {
        const int lfoType = lfoTypeParam ? static_cast<int> (lfoTypeParam->load()) : 0;

        switch (lfoType)
        {
            case 1:
                return static_cast<float> (1.0 - 4.0 * std::abs (lfoPhase - 0.5));

            case 2:
                return lfoPhase < 0.5 ? 1.0f : -1.0f;

            case 0:
            default:
                return static_cast<float> (std::sin (lfoPhase * juce::MathConstants<double>::twoPi));
        }
    }

    static float computeTransient (int osc1Type, int osc2Type, float mix, float envelope, float lastEnvelope) noexcept
    {
        const float onset = juce::jlimit (0.0f, 1.0f, envelope - lastEnvelope);
        const float harmonicBias = ((osc1Type == 0 || osc2Type == 0) ? 0.14f : 0.0f)
                                 + ((osc1Type == 1 || osc2Type == 1) ? 0.08f : 0.0f)
                                 + ((osc1Type == 2 || osc2Type == 2) ? 0.12f : 0.0f);
        const float stereoBlend = 0.6f + std::abs (0.5f - mix);
        return onset * harmonicBias * stereoBlend * 4.0f;
    }

    void updateADSR()
    {
        juce::ADSR::Parameters p;
        p.attack = attackParam ? juce::jmax (0.001f, attackParam->load()) : 0.01f;
        p.decay = decayParam ? juce::jmax (0.001f, decayParam->load()) : 0.3f;
        p.sustain = sustainParam ? juce::jlimit (0.0f, 1.0f, sustainParam->load()) : 0.7f;
        p.release = releaseParam ? juce::jmax (0.001f, releaseParam->load()) : 0.5f;
        adsr.setParameters (p);
    }

    juce::ADSR adsr;
    juce::SmoothedValue<double> smoothedFreq { 440.0 };

    double phase1 = 0.0;
    double phase2 = 0.0;
    double lfoPhase = 0.0;
    float velocity_ = 1.0f;
    float lastEnvelope = 0.0f;
    bool isPrepared = false;
    bool voiceWasActive = false;

    std::atomic<float>* osc1TypeParam = nullptr;
    std::atomic<float>* osc2TypeParam = nullptr;
    std::atomic<float>* oscMixParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* glideParam = nullptr;
    std::atomic<float>* lfoRateParam = nullptr;
    std::atomic<float>* lfoDepthParam = nullptr;
    std::atomic<float>* lfoRoutingParam = nullptr;
    std::atomic<float>* lfoTypeParam = nullptr;
};
