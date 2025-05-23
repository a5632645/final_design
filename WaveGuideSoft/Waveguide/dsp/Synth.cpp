#include "Synth.hpp"
#include "Note.hpp"
#include "MemAttributes.hpp"

namespace dsp {

ThreadSafeCallback gSafeCallback;
CSynthParams SynthParams;
MEM_BSS_ITCM Reverb reverb_;

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

void CSynth::SaveParam(SavedParams& s) {
    auto& p = GetSynthParams();
    s.body.body = p.body.value;
    s.body.bodyType = p.bodyType.value;
    s.body.stretch = p.stretch.value;
    s.body.wetGain = p.wetGain.value;
    s.instrument = instrument_;
    switch (instrument_) {
    case Instrument::Bow: {
        auto& b = s.param.bow;
        b.attack = p.bow.attack.value;
        b.bowPos = p.bow.bowPos.value;
        b.bowSpeed = p.bow.bowSpeed.value;
        b.decay = p.bow.decay.value;
        b.lossFaster = p.bow.lossFaster.value;
        b.lossOutHigh = p.bow.lossOutHigh.value;
        b.lossOutLow = p.bow.lossOutLow.value;
        b.lossTructionHigh = p.bow.lossTructionHigh.value;
        b.lossTructionlow = p.bow.lossTructionlow.value;
        b.noise = p.bow.noise.value;
        b.noiseLP = p.bow.noiseLP.value;
        b.offset = p.bow.offset.value;
        b.reflectMax = p.bow.reflectMax.value;
        b.reflectMin = p.bow.reflectMin.value;
        b.release = p.bow.release.value;
        b.slope = p.bow.slope.value;
        b.tremoloAttack = p.bow.tremoloAttack.value;
        b.tremoloControl = p.bow.tremoloControl.value;
        b.tremoloDepth = p.bow.tremoloDepth.value;
        b.tremoloRate = p.bow.tremoloRate.value;
        b.vibrateAttack = p.bow.vibrateAttack.value;
        b.vibrateControl = p.bow.vibrateControl.value;
        b.vibrateDepth = p.bow.vibrateDepth.value;
        b.vibrateRate = p.bow.vibrateRate.value;
    }
        break;
    case Instrument::Reed: {
        auto& r = s.param.reed;
        r.inhall = p.reed.inhalling.value;
        r.active = p.reed.active.value;
        r.blend = p.reed.blend.value;
        r.noiseGain = p.reed.noiseGain.value;
        r.lossGain = p.reed.lossGain.value;
        r.attack = p.reed.attack.value;
        r.release = p.reed.release.value;
        r.airGain = p.reed.airGain.value;
        r.lossLP = p.reed.lossLP.value;
        r.lossHp = p.reed.lossHP.value;
        r.lossFaster = p.reed.lossFaster.value;
        r.vibrateControl = p.reed.vibrateControl.value;
        r.virbrateDepth = p.reed.vibrateDepth.value;
        r.virbrateRate = p.reed.vibrateRate.value;
        r.virbrateAttack = p.reed.vibrateAttack.value;
        r.tremoloControl = p.reed.tremoloControl.value;
        r.tremoloDepth = p.reed.tremoloDepth.value;
        r.tremoloRate = p.reed.tremoloRate.value;
        r.tremoloAttack = p.reed.tremoloAttack.value;
        r.loopGainAdd = p.reed.loopGainAdd.value;
        r.blendAdd = p.reed.blendAdd.value;
    }
        break;
    case Instrument::String: {
        auto& ss = s.param.string;
        ss.decay = p.string.decay.value;
        ss.exciFaster = p.string.exciFaster.value;
        ss.dispersion = p.string.dispersion.value;
        ss.pos = p.string.pos.value;
        ss.color = p.string.color.value;
        ss.lossFaster = p.string.lossFaster.value;
        ss.lossTructionLow = p.string.lossTructionlow.value;
        ss.lossTructionHigh = p.string.lossTructionHigh.value;
        ss.lossOutLow = p.string.lossOutLow.value;
        ss.lossOutHigh = p.string.lossOutHigh.value;
        ss.virbrateDepth = p.string.vibrateDepth.value;
        ss.exciTructionLow = p.string.exciTructionlow.value;
        ss.exciTructionHigh = p.string.exciTructionHigh.value;
        ss.exciOutLow = p.string.exciOutLow.value;
        ss.exciOutHigh = p.string.exciOutHigh.value;
        ss.posAdd = p.string.posAdd.value;
    }
        break;
    }
}

void CSynth::LoadParam(const SavedParams& s) {
    uint32_t inst_id = static_cast<uint32_t>(s.instrument);
    uint32_t max_inst = static_cast<uint32_t>(Instrument::kNumInstruments);
    if (inst_id >= max_inst) return;

    auto& p = GetSynthParams();
    p.body.SetValue(s.body.body);
    p.bodyType.SetValue(s.body.bodyType);
    p.stretch.SetValue(s.body.stretch);
    p.wetGain.SetValue(s.body.wetGain);
    SetInstrument(s.instrument);
    switch (instrument_) {
    case Instrument::Bow: {
        auto& b = s.param.bow;
        p.bow.attack.SetValue(b.attack);
        p.bow.bowPos.SetValue(b.bowPos);
        p.bow.bowSpeed.SetValue(b.bowSpeed);
        p.bow.decay.SetValue(b.decay);
        p.bow.lossFaster.SetValue(b.lossFaster);
        p.bow.lossOutHigh.SetValue(b.lossOutHigh);
        p.bow.lossOutLow.SetValue(b.lossOutLow);
        p.bow.lossTructionHigh.SetValue(b.lossTructionHigh);
        p.bow.lossTructionlow.SetValue(b.lossTructionlow);
        p.bow.noise.SetValue(b.noise);
        p.bow.noiseLP.SetValue(b.noiseLP);
        p.bow.offset.SetValue(b.offset);
        p.bow.reflectMax.SetValue(b.reflectMax);
        p.bow.reflectMin.SetValue(b.reflectMin);
        p.bow.release.SetValue(b.release);
        p.bow.slope.SetValue(b.slope);
        p.bow.tremoloAttack.SetValue(b.tremoloAttack);
        p.bow.tremoloControl.SetValue(b.tremoloControl);
        p.bow.tremoloDepth.SetValue(b.tremoloDepth);
        p.bow.tremoloRate.SetValue(b.tremoloRate);
        p.bow.vibrateAttack.SetValue(b.vibrateAttack);
        p.bow.vibrateControl.SetValue(b.vibrateControl);
        p.bow.vibrateDepth.SetValue(b.vibrateDepth);
        p.bow.vibrateRate.SetValue(b.vibrateRate);
    }
        break;
    case Instrument::Reed: {
        auto& r = s.param.reed;
        p.reed.inhalling.SetValue(r.inhall);
        p.reed.active.SetValue(r.active);
        p.reed.blend.SetValue(r.blend);
        p.reed.noiseGain.SetValue(r.noiseGain);
        p.reed.lossGain.SetValue(r.lossGain);
        p.reed.attack.SetValue(r.attack);
        p.reed.release.SetValue(r.release);
        p.reed.airGain.SetValue(r.airGain);
        p.reed.lossLP.SetValue(r.lossLP);
        p.reed.lossHP.SetValue(r.lossHp);
        p.reed.lossFaster.SetValue(r.lossFaster);
        p.reed.vibrateControl.SetValue(r.vibrateControl);
        p.reed.vibrateDepth.SetValue(r.virbrateDepth);
        p.reed.vibrateRate.SetValue(r.virbrateRate);
        p.reed.vibrateAttack.SetValue(r.virbrateAttack);
        p.reed.tremoloControl.SetValue(r.tremoloControl);
        p.reed.tremoloDepth.SetValue(r.tremoloDepth);
        p.reed.tremoloRate.SetValue(r.tremoloRate);
        p.reed.tremoloAttack.SetValue(r.tremoloAttack);
        p.reed.loopGainAdd.SetValue(r.loopGainAdd);
        p.reed.blendAdd.SetValue(r.blendAdd);
    }
        break;
    case Instrument::String: {
        auto& ss = s.param.string;
        p.string.decay.SetValue(ss.decay);
        p.string.exciFaster.SetValue(ss.exciFaster);
        p.string.dispersion.SetValue(ss.dispersion);
        p.string.pos.SetValue(ss.pos);
        p.string.color.SetValue(ss.color);
        p.string.lossFaster.SetValue(ss.lossFaster);
        p.string.lossTructionlow.SetValue(ss.lossTructionLow);
        p.string.lossTructionHigh.SetValue(ss.lossTructionHigh);
        p.string.lossOutLow.SetValue(ss.lossOutLow);
        p.string.lossOutHigh.SetValue(ss.lossOutHigh);
        p.string.vibrateDepth.SetValue(ss.virbrateDepth);
        p.string.exciTructionlow.SetValue(ss.exciTructionLow);
        p.string.exciTructionHigh.SetValue(ss.exciTructionHigh);
        p.string.exciOutLow.SetValue(ss.exciOutLow);
        p.string.exciOutHigh.SetValue(ss.exciOutHigh);
        p.string.posAdd.SetValue(ss.posAdd);
    }
        break;
    }
}

void CSynth::BindParamsFlute(CSynthParams& param)
{
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
