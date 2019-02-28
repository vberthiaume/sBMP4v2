/*
  ==============================================================================

    Helpers.h
    Created: 21 Jan 2019 11:52:58am
    Author:  Haake

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#ifndef CPU_USAGE
    #define CPU_USAGE 0
#endif

struct Selection
{
    Selection() = default;
    Selection (int selection) : curSelection (selection) {}

    int curSelection;

    virtual int getLastSelectionIndex() = 0;
    virtual bool isNullSelectionAllowed() = 0;
};

struct OscShape : public Selection
{
    enum
    {
        none = 0,
        saw,
        sawTri,
        triangle,
        pulse,
        total
    };

    int getLastSelectionIndex() override { return total - 1; }
    bool isNullSelectionAllowed() override { return true; }
};

struct LfoShape : public Selection
{
    enum
    {
        triangle = 0,
        saw,
        //revSaw,
        square,
        random,
        total
    };

    int getLastSelectionIndex() override { return total - 1; }
    bool isNullSelectionAllowed() override { return false; }
};

struct LfoDest : public Selection
{
    enum
    {
        osc1Freq = 0,
        osc2Freq,
        filterCurOff,
        filterResonance,
        total
    };

    int getLastSelectionIndex() override { return total - 1; }
    bool isNullSelectionAllowed() override { return false; }
};

namespace Constants
{
    static const auto defaultOscLevel = .4f;

    static const auto defaultFilterCutoff = 1000.f;
    static const auto defaultFilterResonance = .5f;

    static const auto defaultAmpA = .1f;
    static const auto defaultAmpD = .1f;
    static const auto defaultAmpS = 1.f;
    static const auto defaultAmpR = .25f;
    static const auto sustainSkewFactor = .5f;
    static const auto ampSkewFactor = .1f;

    static const float defaultLfoFreq = 3.f;
    static const float defaultLfoAmount = 0.f;

    static const float defaultEffectParam1 = 0.f;

    static const NormalisableRange<float> dBRange = {-12.f, 12.f};
    static const NormalisableRange<float> sliderRange = {0.f, 1.f};
    static const NormalisableRange<float> sustainRange = {std::numeric_limits<float>::epsilon(), 1.f, 0.f, sustainSkewFactor};
    static const NormalisableRange<float> ampRange = {std::numeric_limits<float>::epsilon(), 25.f, 0.f, ampSkewFactor};

    static const NormalisableRange<float> hzRange = {0.1f, 18000.f};
    static const NormalisableRange<float> lfoRange = {0.1f, 10.f};
    static const NormalisableRange<float> lfoNoteRange = {0.f, 16.f};

    //Sets the base frequency of Oscillator 1 or 2 over a 9-octave
    //range from 16 Hz to 8KHz (when used with the Transpose buttons). Adjustment is in semitones.
    static const NormalisableRange<int> midiNoteRange = {12, 120};   //actual midi note range is (0,127), but rev2, at least for oscilators is C0(0) to C10(120)
    static const NormalisableRange<float> pitchWheelNoteRange = {-7.f, 7.f};

    enum
    {
        oscShapeRadioGroupId = 1,

        numVoices = 2,
        defaultOscMidiNote = 48,    //C2 on rev2, used to be 36 for some reason
        middleCMidiNote = 60,       //C3 on rev2

        defaultOscShape = (int) OscShape::saw + 1,
        defaultLfoShape = (int) LfoShape::triangle,
        defaultLfoDest = (int) LfoDest::filterCurOff
    };
}

namespace sBMP4AudioProcessorIDs
{
    const String osc1ShapeID    = "osc1ShapeID";
    const String osc1FreqID     = "osc1FreqID";
    const String osc2ShapeID    = "osc2ShapeID";
    const String osc2FreqID     = "osc2FreqID";

    const String filterCutoffID     = "filterCutoffID";
    const String filterResonanceID  = "filterResonanceID";

    const String ampAttackID    = "ampAttackID";
    const String ampDecayID     = "ampDecayID";
    const String ampSustainID   = "ampSustainID";
    const String ampReleaseID   = "ampReleaseID";

    const String lfoShapeID     = "lfoShapeID";
    const String lfoDestID      = "lfoDestID";
    const String lfoFreqID      = "lfoFreqID";
    const String lfoAmountID    = "lfoAmountID";

    const String effectID       = "effectID";
    const String effectParam1ID = "effectParam1ID";
    const String effectParam2ID = "effectParam2ID";
}

namespace sBMP4AudioProcessorNames
{
    const String oscGroupDesc = "OSCILLATORS";
    const String osc1ShapeDesc = "SHAPE";
    const String osc2ShapeDesc = "SHAPE";
    const String osc1FreqSliderDesc = "OSC 1 FREQ";
    const String osc2FreqSliderDesc = "OSC 2 FREQ";

    const String filterGroupDesc = "LOW-PASS FILTER";
    const String filterCutoffSliderDesc = "CUTOFF";
    const String filterResonanceSliderDesc = "RESONANCE";

    const String ampGroupDesc = "AMPLIFIER";
    const String ampAttackSliderDesc = "ATTACK";
    const String ampDecaySliderDesc = "DECAY";
    const String ampSustainSliderDesc = "SUSTAIN";
    const String ampReleaseSliderDesc = "RELEASE";

    const String lfoGroupDesc = "LFO";
    const String lfoShapeDesc = "SHAPE";
    const String lfoDestDesc = "DEST";
    const String lfoFreqSliderDesc = "FREQUENCY";
    const String lfoAmountSliderDesc = "AMOUNT";

    const String effectGroupDesc = "EFFECT";
    const String effectParam1Desc = "PARAM 1";
    const String effectParam2Desc = "PARAM 2";
}

namespace sBMP4AudioProcessorChoices
{
    const String oscShape0 = "None";
    const String oscShape1 = "Sawtooth";
    const String oscShape2 = "Saw + Tri";
    const String oscShape3 = "Triangle";
    const String oscShape4 = "Pulse";

    const String lfoShape0 = "Triangle";
    const String lfoShape1 = "Sawtooth";
    //const String lfoShape2 = "Rev Saw";
    const String lfoShape3 = "Square";
    const String lfoShape4 = "Random";

    const String lfoDest0 = "Osc1 Freq";
    const String lfoDest1 = "Osc2 Freq";
    const String lfoDest2 = "Cutoff";
    const String lfoDest3 = "Resonance";

    const String effectChoices0 = "Reverb";
    const String effectChoices1 = "None";
}

inline bool getVarAsBool (const ValueTree& v, const Identifier& id) { return static_cast<bool> (v.getProperty (id)); }
inline int getVarAsInt (const ValueTree& v, const Identifier& id) { return static_cast<int> (v.getProperty (id)); }
inline int64 getVarAsInt64 (const ValueTree& v, const Identifier& id) { return static_cast<int64> (v.getProperty (id)); }
inline double getVarAsDouble (const ValueTree& v, const Identifier& id) { return static_cast<double> (v.getProperty (id)); }
inline float getVarAsFloat (const ValueTree& v, const Identifier& id) { return static_cast<float> (getVarAsDouble (v, id)); }
inline String getVarAsString (const ValueTree& v, const Identifier& id) { return v.getProperty (id).toString(); }
inline Identifier getVarAsIdentifier (const ValueTree& v, const Identifier& id) { return static_cast<Identifier> (v.getProperty (id)); }

struct Helpers
{
    static Image getImage (const void* imageData, const int dataSize)
    {
        return ImageCache::getFromMemory (imageData, dataSize);
    }

    static Drawable* getDrawable (const void* imageData, const int dataSize)
    {
        return Drawable::createFromImageData (imageData, dataSize);
    }

    static float getRangedParamValue (AudioProcessorValueTreeState& state, StringRef id)
    {
        auto param = state.getParameter (id);
        return param == nullptr ? 0.f : param->convertFrom0to1 (param->getValue());
    }

    static double getDoubleMidiNoteInHertz (const double noteNumber, const double frequencyOfA = 440.0) noexcept
    {
        return frequencyOfA * std::pow (2.0, (noteNumber - 69.0) / 12.0);
    }

    static float getFloatMidiNoteInHertz (const float noteNumber, const float frequencyOfA = 440.0) noexcept
    {
        return frequencyOfA * (float) std::pow (2.0, (noteNumber - 69.0) / 12.0);
    }
};

#if 0
auto addChoicesToComboBox = [this](Array<ComboBox*> combos, Array<StringRef> comboIDs)
{
    jassert (combos.size() == comboIDs.size());

    for (int i = 0; i < combos.size(); ++i)
    {
        auto comboParam = (AudioParameterChoice*) processor.state.getParameter (comboIDs[i]);
        jassert (comboParam != nullptr);

        int j = 1;
        for (auto choice : comboParam->choices)
            combos[i]->addItem (choice, j++);

        combos[i]->setSelectedItemIndex (0, sendNotification);
    }
};

addChoicesToComboBox ({ /*&oscCombo,*/ &lfoCombo}, { /*osc1ShapeID, */lfoShapeID});
#endif
