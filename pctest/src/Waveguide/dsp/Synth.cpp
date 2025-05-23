#include "Synth.hpp"
#include "Note.hpp"

namespace dsp {

ThreadSafeCallback gSafeCallback;
CSynthParams SynthParams;
Reverb reverb_;

void CSynth::Init(uint32_t sampleRate) {
    DelayAllocator::Init();
    string_.Init(sampleRate);
    BindParamsString(GetSynthParams());
    reed_.Init(sampleRate);
    BindParamsFlute(GetSynthParams());
    bowed_.Init(sampleRate);
    BindParamsBow(GetSynthParams());
    reverb_.Init(sampleRate);
    BindParamReverb(GetSynthParams());
    body_.Init(sampleRate);
    BindParamsBody(GetSynthParams());

    for (uint32_t i = 0; i < string_.kNumPolyonic; ++i) {
        auto* delay1 = DelayAllocator::GetDelayLine();
        auto* delay2 = DelayAllocator::GetDelayLine();
        string_.GetNotes()[i].SetDelayLineRef(delay1, delay2);
        reed_.GetNotes()[i].SetDelayLineRef(delay1);
        bowed_.GetNotes()[i].SetDelayLineRef(delay1, delay2);
    }
}

void CSynth::NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    switch (instrument_) {
    case Instrument::Bow:
        bowed_.NoteOn(channel, note, velocity);
        break;
    case Instrument::Reed:
        reed_.NoteOn(channel, note, velocity);
        break;
    case Instrument::String:
        string_.NoteOn(channel, note, velocity);
        break;
    }
}

void CSynth::NoteOff(uint8_t note) {
    switch (instrument_) {
    case Instrument::Bow:
        bowed_.NoteOff(note);
        break;
    case Instrument::Reed:
        reed_.NoteOff(note);
        break;
    case Instrument::String:
        string_.NoteOff(note);
        break;
    }
}

void CSynth::Process(std::span<float> buffer, std::span<float> auxBuffer) {
    switch (instrument_) {
    case Instrument::Bow:
        bowed_.Process(buffer, auxBuffer);
        break;
    case Instrument::Reed:
        reed_.Process(buffer, auxBuffer);
        break;
    case Instrument::String:
        string_.Process(buffer, auxBuffer);
        break;
    }
    body_.Process(buffer, auxBuffer);
    reverb_.Process(buffer, auxBuffer);
}

void CSynth::SetInstrument(Instrument instr) {
    if (instr != instrument_) {
        switch (instrument_) {
        case Instrument::Bow:
            bowed_.ForceStopAll();
            break;
        case Instrument::Reed:
            reed_.ForceStopAll();
            break;
        case Instrument::String:
            string_.ForceStopAll();
            break;
        }
        instrument_ = instr;
    }
}

void CSynth::BindParamsFlute(CSynthParams& param) {
    param.reed.lossHP.SetCallback([]{
        
        auto v = Synth.GetSynthParams().reed.lossHP.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetLossHP(v);
        }
    });
    param.reed.lossGain.SetCallback([]{
        
        auto v = Synth.GetSynthParams().reed.lossGain.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetLossGain(v);
        }
    });
    param.reed.lossLP.SetCallback([]{
        
        auto v = Synth.GetSynthParams().reed.lossLP.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetLossLP(v);
        }
    });
    param.reed.noiseGain.SetCallback([]{
        
        auto v = Synth.GetSynthParams().reed.noiseGain.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetNoiseGain(v);
        }
    });
    param.reed.inhalling.SetCallback([]{
        auto v = Synth.GetSynthParams().reed.inhalling.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetInhalingOffset(v);
        }
    });
    param.reed.active.SetCallback([]{
        auto v = Synth.GetSynthParams().reed.active.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetActiveOffset(v);
        }
    });
    param.reed.blend.SetCallback([]{
        auto v = Synth.GetSynthParams().reed.blend.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetBlend(v);
        }
    });
    param.reed.attack.SetCallback([]{
        
        auto v = Synth.GetSynthParams().reed.attack.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetAttack(v);
        }
    });
    param.reed.release.SetCallback([]{
        
        auto v = Synth.GetSynthParams().reed.release.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetRelease(v);
        }
    });
    param.reed.airGain.SetCallback([]{
        auto v = Synth.GetSynthParams().reed.airGain.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetAirGain(v);
        }
    });
    param.reed.lossFaster.SetCallback([]{
        auto v = Synth.GetSynthParams().reed.lossFaster.Get();
        for (auto& note : Synth.reed_.GetNotes()) {
            note.SetLossFaster(v);
        }
    });
    param.reed.tremoloAttack.SetCallback([]{
        auto v = Synth.GetSynthParams().reed.tremoloAttack.Get();
        for (auto& n : Synth.reed_.GetNotes()) {
            n.SetTremoloAttack(v);
        }
    });
    param.reed.vibrateAttack.SetCallback([]{
        auto v = Synth.GetSynthParams().reed.vibrateAttack.Get();
        for (auto& n : Synth.reed_.GetNotes()) {
            n.SetVibrateAttack(v);
        }
    });
}

void CSynth::BindParamsString(CSynthParams& param) {
    param.string.decay.SetCallback([] {
        auto& p = Synth.GetSynthParams().string;
        auto notes = Synth.string_.GetNotes();
        auto v = p.decay.Get();
        for (auto& note : notes) {
            note.SetDecay(v);
        }
    });
    param.string.dispersion.SetCallback([] {
        auto& p = Synth.GetSynthParams().string;
        auto notes = Synth.string_.GetNotes();
        auto v = p.dispersion.Get();
        for (auto& note : notes) {
            note.SetDispersion(v);
        }
    });
    param.string.pos.SetCallback([] {
        auto& p = Synth.GetSynthParams().string;
        auto notes = Synth.string_.GetNotes();
        auto v = p.pos.Get();
        for (auto& note : notes) {
            note.SetPluckPosition(v);
        }
    });
    param.string.color.SetCallback([] {
        auto& p = Synth.GetSynthParams().string;
        auto v = p.color.Get();
        auto notes = Synth.string_.GetNotes();
        for (auto& note : notes) {
            note.SetColor(v);
        }
    });
    param.string.lossFaster.SetCallback([] {
        auto& p = Synth.GetSynthParams().string;
        auto v = p.lossFaster.Get();
        auto notes = Synth.string_.GetNotes();
        for (auto& note : notes) {
            note.SetLossFaster(v);
        }
    });
    param.string.exciFaster.SetCallback([] {
        auto& p = Synth.GetSynthParams().string;
        auto v = p.exciFaster.Get();
        auto notes = Synth.string_.GetNotes();
        for (auto& note : notes) {
            note.SetExciterFaster(v);
        }
    });
}

void CSynth::BindParamsBow(CSynthParams& param) {
    param.bow.bowPos.SetCallback([] {
        auto& p = Synth.GetSynthParams().bow;
        for (auto& note : Synth.bowed_.GetNotes()) {
            note.SetBowPosition(p.bowPos.Get());
        }
    });
    param.bow.bowSpeed.SetCallback([] {
        auto& p = Synth.GetSynthParams().bow;
        for (auto& note : Synth.bowed_.GetNotes()) {
            note.SetBowSpeed(p.bowSpeed.Get());
        }
    });
    param.bow.lossGain.SetCallback([] {
        auto& p = Synth.GetSynthParams().bow;
        for (auto& note : Synth.bowed_.GetNotes()) {
            note.SetLossGain(p.lossGain.Get());
        }
    });
    param.bow.offset.SetCallback([] {
        auto& p = Synth.GetSynthParams().bow;
        for (auto& note : Synth.bowed_.GetNotes()) {
            note.SetBowTableOffset(p.offset.Get());
        }
    });
    param.bow.reflectMax.SetCallback([] {
        auto& p = Synth.GetSynthParams().bow;
        for (auto& note : Synth.bowed_.GetNotes()) {
            note.SetBowTableMax(p.reflectMax.Get());
        }
    });
    param.bow.reflectMin.SetCallback([] {
        auto& p = Synth.GetSynthParams().bow;
        for (auto& note : Synth.bowed_.GetNotes()) {
            note.SetBowTableMin(p.reflectMin.Get());
        }
    });
    param.bow.slope.SetCallback([] {
        auto& p = Synth.GetSynthParams().bow;
        for (auto& note : Synth.bowed_.GetNotes()) {
            note.SetBowTableSlope(p.slope.Get());
        }
    });
    param.bow.lossFaster.SetCallback([] {
        auto& p = Synth.GetSynthParams().bow;
        for (auto& note : Synth.bowed_.GetNotes()) {
            note.SetLossFaster(p.lossFaster.Get());
        }
    });
    param.bow.tremoloAttack.SetCallback([]{
        auto v = Synth.GetSynthParams().bow.tremoloAttack.Get();
        for (auto& n : Synth.bowed_.GetNotes()) {
            n.SetTremoloAttack(v);
        }
    });
    param.bow.vibrateAttack.SetCallback([]{
        auto v = Synth.GetSynthParams().bow.vibrateAttack.Get();
        for (auto& n : Synth.bowed_.GetNotes()) {
            n.SetVibrateAttack(v);
        }
    });
    param.bow.noiseLP.SetCallback([]{
        auto v = Synth.GetSynthParams().bow.noiseLP.Get();
        for (auto& n : Synth.bowed_.GetNotes()) {
            n.SetNoiseLP(v);
        }
    });
    param.bow.attack.SetCallback([]{
        auto v = Synth.GetSynthParams().bow.attack.Get();
        for (auto& n : Synth.bowed_.GetNotes()) {
            n.SetAttack(v);
        }
    });
    param.bow.release.SetCallback([]{
        auto v = Synth.GetSynthParams().bow.release.Get();
        for (auto& n : Synth.bowed_.GetNotes()) {
            n.SetRelease(v);
        }
    });
}

void CSynth::BindParamReverb(CSynthParams& param) {
    param.reverb.decay.SetCallback([] {
        reverb_.SetDecayTime(SynthParams.reverb.decay.Get());
    });
    param.reverb.interval.SetCallback([] {
        reverb_.NewVelvetNoise(SynthParams.reverb.interval.Get());
    });
    param.reverb.lossLP.SetCallback([] {
        auto st = SynthParams.reverb.lossLP.Get();
        auto freq = Note::Midi2Frequency(st);
        reverb_.SetLowpassFreq(freq);
    });
    param.reverb.rate.SetCallback([] {
        reverb_.SetQuadOscRate(SynthParams.reverb.rate.Get());
    });
    param.reverb.drywet.SetCallback([] {
        reverb_.SetDryWet(SynthParams.reverb.drywet.Get());
    });
    param.reverb.earlyRefl.SetCallback([] {
        reverb_.SetEarlyReflectionSize(SynthParams.reverb.earlyRefl.Get());
    });
    param.reverb.depth.SetCallback([] {
        reverb_.SetModulationDepth(SynthParams.reverb.depth.Get());
    });
    param.reverb.size.SetCallback([] {
        reverb_.SetSize(SynthParams.reverb.size.Get());
    });
    param.reverb.chrousDept.SetCallback([] {
        reverb_.SetChrousDepth(SynthParams.reverb.chrousDept.Get());
    });
    param.reverb.chrousRate.SetCallback([] {
        reverb_.SetChrousRate(SynthParams.reverb.chrousRate.Get());
    });
}

void CSynth::BindParamsBody(CSynthParams& param) {
    param.body.SetCallback([] {
        Synth.body_.SetEnabled(SynthParams.body.Get());
    });
    param.bodyType.SetCallback([] {
        Synth.body_.SetBodyType(static_cast<BodyEnum>(SynthParams.bodyType.Get()));
    });
    param.wetGain.SetCallback([] {
        Synth.body_.SetWetGain(SynthParams.wetGain.Get());
    });
    param.stretch.SetCallback([] {
        Synth.body_.SetStretch(SynthParams.stretch.Get());
    });
}

} // namespace dsp
