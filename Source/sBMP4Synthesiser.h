/*
  ==============================================================================

    sBMP4Synthesiser.h
    Created: 29 Jan 2019 10:59:23am
    Author:  Haake

  ==============================================================================
*/

#pragma once

#include "SineWaveVoice.h"
#include "Helpers.h"

class sBMP4Synthesiser : public Synthesiser, public AudioProcessorValueTreeState::Listener
{
public:
    sBMP4Synthesiser()
    {
        for (auto i = 0; i < numVoices; ++i)
            addVoice (new sBMP4Voice (i));

        setNoteStealingEnabled (true);

        addSound (new SineWaveSound());
    }

    void prepare (const dsp::ProcessSpec& spec) noexcept
    {
        setCurrentPlaybackSampleRate (spec.sampleRate);

        for (auto* v : voices)
            dynamic_cast<sBMP4Voice*> (v)->prepare (spec);

        fxChain.prepare (spec);
    }

    void parameterChanged (const String& parameterID, float newValue) override
    {
        if (parameterID == sBMP4AudioProcessorIDs::osc1FreqID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOsc1Tuning ((int) newValue); }, newValue);

        else if (parameterID == sBMP4AudioProcessorIDs::osc2FreqID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOsc2Tuning ((int) newValue); }, newValue);

        else if (parameterID == sBMP4AudioProcessorIDs::osc1ShapeID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOsc1Shape ((int) newValue); }, newValue);

        else if (parameterID == sBMP4AudioProcessorIDs::osc2ShapeID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOsc2Shape ((int) newValue); }, newValue);

        else if (parameterID == sBMP4AudioProcessorIDs::ampAttackID
                || parameterID == sBMP4AudioProcessorIDs::ampDecayID
                || parameterID == sBMP4AudioProcessorIDs::ampSustainID
                || parameterID == sBMP4AudioProcessorIDs::ampReleaseID)
            for (auto voice : voices)
                dynamic_cast<sBMP4Voice*> (voice)->setAmpParam (parameterID, newValue);

        else if (parameterID == sBMP4AudioProcessorIDs::lfoShapeID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setLfoShape ((int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::lfoDestID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setLfoDest ((int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::lfoFreqID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setLfoFreq (newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::lfoAmountID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setLfoAmount (newValue); }, newValue);

        else if (parameterID == sBMP4AudioProcessorIDs::filterCutoffID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setFilterCutoff (newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::filterResonanceID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setFilterResonance (newValue); }, newValue);
    }

    using VoiceOperation = std::function<void (sBMP4Voice*, float)>;
    void applyToAllVoices (VoiceOperation operation, float newValue)
    {
        for (auto voice : voices)
            operation (dynamic_cast<sBMP4Voice*> (voice), newValue);
    }

private:

    enum
    {
        reverbIndex
    };

    void renderVoices (AudioBuffer<float>& outputAudio, int startSample, int numSamples) override
    {
        for (auto* voice : voices)
            voice->renderNextBlock (outputAudio, startSample, numSamples);

        //auto block = dsp::AudioBlock<float> (outputAudio);
        //auto blockToUse = block.getSubBlock ((size_t) startSample, (size_t) numSamples);
        //auto contextToUse = dsp::ProcessContextReplacing<float> (blockToUse);
        //fxChain.process (contextToUse);
    }

    dsp::ProcessorChain<dsp::Reverb> fxChain;
};
