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
            addVoice (new sBMP4Voice (i, &activeVoices, &voicesBeingKilled));

        addSound (new sBMP4Sound());
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
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscFreq (sBMP4Voice::osc1Index, (int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc2FreqID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscFreq (sBMP4Voice::osc2Index, (int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc1TuningID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscTuning (sBMP4Voice::osc1Index, newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc2TuningID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscTuning (sBMP4Voice::osc2Index, newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc1ShapeID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscShape (sBMP4Voice::osc1Index, (int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::osc2ShapeID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscShape (sBMP4Voice::osc2Index, (int) newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::oscSubID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscSub (newValue); }, newValue);
        else if (parameterID == sBMP4AudioProcessorIDs::oscMixID)
            applyToAllVoices ([](sBMP4Voice* voice, float newValue) { voice->setOscMix (newValue); }, newValue);

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
        else
            jassertfalse;
    }

    using VoiceOperation = std::function<void (sBMP4Voice*, float)>;
    void applyToAllVoices (VoiceOperation operation, float newValue)
    {
        for (auto voice : voices)
            operation (dynamic_cast<sBMP4Voice*> (voice), newValue);
    }

    void noteOn (const int midiChannel, const int midiNoteNumber, const float velocity) override
    {
        //we have too many voices being played, just forget about this one
        if (voicesBeingKilled.size() >= numVoicesSoft)
            return;

        {
            const ScopedLock sl (lock);

            auto numVoicesToKill = (int) activeVoices.size() - (int) voicesBeingKilled.size() - numVoicesSoft;

            while (numVoicesToKill-- > 0)
                if (auto voice = dynamic_cast<sBMP4Voice*> (findFreeVoiceOrSteal (*sounds.begin(), midiChannel, midiNoteNumber)))
                    voice->killNote();
        }

        //only start new voices if we have room
        if (activeVoices.size() < numVoices)
            Synthesiser::noteOn (midiChannel, midiNoteNumber, velocity);
        //else
        //    jassertfalse;
    }

    SynthesiserVoice* findFreeVoiceOrSteal (SynthesiserSound* soundToPlay, int /*midiChannel*/, int midiNoteNumber) const
    {
        // This voice-stealing algorithm applies the following heuristics:
        // - Re-use the oldest notes first
        // - Protect the lowest & topmost notes, even if sustained, but not if they've been released.

        // apparently you are trying to render audio without having any voices...
        jassert (! voices.isEmpty());

        // These are the voices we want to protect (ie: only steal if unavoidable)
        SynthesiserVoice* low = nullptr; // Lowest sounding note, might be sustained, but NOT in release phase
        SynthesiserVoice* top = nullptr; // Highest sounding note, might be sustained, but NOT in release phase

        // this is a list of voices we can steal, sorted by how long they've been running
        Array<SynthesiserVoice*> usableVoices;
        usableVoices.ensureStorageAllocated (voices.size());

        //@TODO so we only ever try to kill voices in the soft range... is that a problem?
        for (int i = 0; i < numVoices; ++i)
        {
            auto voice = dynamic_cast<sBMP4Voice*> (voices[i]);
            if (voice->canPlaySound (soundToPlay))
            {
                //do not try to steal/kill this voice if it is currently being killed
                if (/*! voice->isVoiceActive() || */voicesBeingKilled.find (voice->getVoiceId()) != voicesBeingKilled.end())
                    continue;

                usableVoices.add (voice);

                // NB: Using a functor rather than a lambda here due to scare-stories about
                // compilers generating code containing heap allocations..
                struct Sorter
                {
                    bool operator() (const SynthesiserVoice* a, const SynthesiserVoice* b) const noexcept { return a->wasStartedBefore (*b); }
                };

                std::sort (usableVoices.begin(), usableVoices.end(), Sorter());

                if (! voice->isPlayingButReleased()) // Don't protect released notes
                {
                    auto note = voice->getCurrentlyPlayingNote();

                    if (low == nullptr || note < low->getCurrentlyPlayingNote())
                        low = voice;

                    if (top == nullptr || note > top->getCurrentlyPlayingNote())
                        top = voice;
                }
            }
        }

        // Eliminate pathological cases (ie: only 1 note playing): we always give precedence to the lowest note(s)
        if (top == low)
            top = nullptr;

        // The oldest note that's playing with the target pitch is ideal..
        for (auto* voice : usableVoices)
            if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
                return voice;

        // Oldest voice that has been released (no finger on it and not held by sustain pedal)
        for (auto* voice : usableVoices)
            if (voice != low && voice != top && voice->isPlayingButReleased())
                return voice;

        // Oldest voice that doesn't have a finger on it:
        for (auto* voice : usableVoices)
            if (voice != low && voice != top && ! voice->isKeyDown())
                return voice;

        // Oldest voice that isn't protected
        for (auto* voice : usableVoices)
            if (voice != low && voice != top)
                return voice;

        // We've only got "protected" voices now: lowest note takes priority
        jassert (low != nullptr);

        // Duophonic synth: give priority to the bass note:
        if (top != nullptr)
            return top;

        return low;
    }

private:

    enum
    {
        reverbIndex
    };

    //@TODO: make this into a bit mask thing? is there any concurency issues here?
    //@TODO Should I have voices on different threads?
    std::set<int> activeVoices{};
    std::set<int> voicesBeingKilled{};

    dsp::ProcessorChain<dsp::Reverb> fxChain;
};
