#pragma once
#include <cstdint>
#include <span>
#include "params.hpp"
#include "Reed.hpp"
#include "PluckString.hpp"
#include "Bowed.hpp"
#include "PolySynth.hpp"
#include "Reverb.hpp"
#include "Body.hpp"

namespace dsp {

class CSynth {
public:
    enum class Instrument : uint8_t { String = 0, Reed, Bow, kNumInstruments };

    void Init(uint32_t sampleRate);
    void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void NoteOff(uint8_t note);
    void Process(std::span<float> buffer, std::span<float> auxBuffer);

    void SetInstrument(Instrument instr);
    Instrument GetInstrument() const { return instrument_; }

    CSynthParams& GetSynthParams() { return SynthParams; }

    Reed& GetReed() {
        auto used = reed_.GetUsedNotes();
        if (used.size() > 0) {
            return *used[0];
        }
        else {
            return reed_.GetNotes()[0];
        }
    }
    Bowed& GetBowed() {
        auto used = bowed_.GetUsedNotes();
        if (used.size() > 0) {
            return *used[0];
        }
        else {
            return bowed_.GetNotes()[0];
        }
    }
private:
    void BindParamsFlute(CSynthParams& param);
    void BindParamsString(CSynthParams& param);
    void BindParamsBow(CSynthParams& param);
    void BindParamReverb(CSynthParams& param);
    void BindParamsBody(CSynthParams& param);

    PolySynth<PluckString> string_{};
    PolySynth<Bowed> bowed_{};
    PolySynth<Reed> reed_{};
    Instrument instrument_{ Instrument::String };
    Body body_;
};

struct InternalSynth {
    inline static CSynth instance;
};

static CSynth& Synth = InternalSynth::instance;

} // namespace dsp
