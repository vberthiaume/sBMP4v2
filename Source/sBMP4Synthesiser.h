/*
  ==============================================================================

   Copyright (c) 2019 - Vincent Berthiaume

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   sBMP4 IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
            addVoice (new sBMP4Voice (i, &voicesBeingKilled));

        addSound (new sBMP4Sound());

        setMasterGain (defaultMasterGain);
        fxChain.get<masterGainIndex>().setRampDurationSeconds (0.1);
    }

    void prepare (const dsp::ProcessSpec& spec) noexcept
    {
        if (Helpers::areSameSpecs (curSpecs, spec))
            return;

        curSpecs = spec;

        setCurrentPlaybackSampleRate (spec.sampleRate);

        for (auto* v : voices)
            dynamic_cast<sBMP4Voice*> (v)->prepare (spec);

        fxChain.prepare (spec);
    }

    void parameterChanged (const String& parameterID, float newValue) override
    {
        if (parameterID == sBMP4AudioProcessorIDs::osc1FreqID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscFreq (sBMP4Voice::processorId::osc1Index, (int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc2FreqID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscFreq (sBMP4Voice::processorId::osc2Index, (int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc1TuningID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscTuning (sBMP4Voice::processorId::osc1Index, newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc2TuningID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscTuning (sBMP4Voice::processorId::osc2Index, newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc1ShapeID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscShape (sBMP4Voice::processorId::osc1Index, (int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc2ShapeID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscShape (sBMP4Voice::processorId::osc2Index, (int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::oscSubID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscSub (newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::oscMixID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscMix (newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::oscNoiseID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscNoise (newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::oscSlopID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscSlop (newValue); }, newValue);

        else if (parameterID == sBMP4AudioProcessorIDs::ampAttackID
                || parameterID == sBMP4AudioProcessorIDs::ampDecayID
                || parameterID == sBMP4AudioProcessorIDs::ampSustainID
                || parameterID == sBMP4AudioProcessorIDs::ampReleaseID)
            for (auto voice : voices)
                dynamic_cast<sBMP4Voice*> (voice)->setAmpParam (parameterID, newValue);

        else if (parameterID == sBMP4AudioProcessorIDs::filterEnvAttackID
                || parameterID == sBMP4AudioProcessorIDs::filterEnvDecayID
                || parameterID == sBMP4AudioProcessorIDs::filterEnvSustainID
                || parameterID == sBMP4AudioProcessorIDs::filterEnvReleaseID)
            for (auto voice : voices)
                dynamic_cast<sBMP4Voice*> (voice)->setFilterEnvParam (parameterID, newValue);

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

        else if (parameterID == sBMP4AudioProcessorIDs::effectParam1ID || parameterID == sBMP4AudioProcessorIDs::effectParam2ID)
            setEffectParam (parameterID, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::masterGainID)
            setMasterGain (newValue);
        else
            jassertfalse;
    }

    using VoiceOperation = std::function<void (sBMP4Voice*, float)>;
    void applyToAllVoices (VoiceOperation operation, float newValue)
    {
        for (auto voice : voices)
            operation (dynamic_cast<sBMP4Voice*> (voice), newValue);
    }

    void setEffectParam (StringRef parameterID, float newValue)
    {
        if (parameterID == sBMP4AudioProcessorIDs::effectParam1ID)
            reverbParams.roomSize = newValue;
        else if (parameterID == sBMP4AudioProcessorIDs::effectParam2ID)
            reverbParams.wetLevel = newValue;

        fxChain.get<reverbIndex>().setParameters (reverbParams);
    }

    void setMasterGain (float gain)
    {
        fxChain.get<masterGainIndex>().setGainLinear (gain);
    }

    void noteOn (const int midiChannel, const int midiNoteNumber, const float velocity) override
    {
        {
            const ScopedLock sl (lock);

            //don't start new voices in current buffer call if we have filled all voices already.
            //voicesBeingKilled should be reset after each renderNextBlock call
            if (voicesBeingKilled.size() >= numVoices)
                return;
        }

        Synthesiser::noteOn (midiChannel, midiNoteNumber, velocity);
    }

private:

    void renderVoices (AudioBuffer<float>& outputAudio, int startSample, int numSamples) override
    {
        for (auto* voice : voices)
            voice->renderNextBlock (outputAudio, startSample, numSamples);

        auto block = dsp::AudioBlock<float> (outputAudio);
        auto blockToUse = block.getSubBlock ((size_t) startSample, (size_t) numSamples);
        auto contextToUse = dsp::ProcessContextReplacing<float> (blockToUse);
        fxChain.process (contextToUse);
    }

    enum
    {
        reverbIndex,
        masterGainIndex
    };

    //@TODO: make this into a bit mask thing?
    std::set<int> voicesBeingKilled{};

    dsp::ProcessorChain<dsp::Reverb, dsp::Gain<float>> fxChain;

    dsp::Reverb::Parameters reverbParams;

    dsp::ProcessSpec curSpecs{};
};
