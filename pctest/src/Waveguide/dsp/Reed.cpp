#include "Reed.hpp"
#include "Util.hpp"
#include "Note.hpp"
#include "DelayAllocator.hpp"
#include "utli/Lerp.hpp"
#include "params.hpp"

namespace dsp {

void Reed::Init(float sampleRate) {
    pipe_->Init(sampleRate);
    lossLP_.Init(sampleRate);
    envelop_.Init(sampleRate);
    noise_.Init(sampleRate);
    lossHP_.Init(sampleRate);
    sampleRate_ = sampleRate;
    tunningFilter_.Init(sampleRate);
    noiseLP_.Init(sampleRate);
    noiseLP_.SetCutoffHPF(5000);

    vibrateOscPhase_ = 0.0f;
    tremoloOscPhase_ = 0.0f;
    vibrateDelay_.Init(sampleRate / 480);
    tremoloDelay_.Init(sampleRate / 480);
}

bool Reed::Process(std::span<float> buffer, std::span<float> auxBuffer) {
    maxSample_ = 0.0f;
    UpdateDelayLen();
    for (auto& s : buffer) {
        s = ProcessSingle();
    }
    return maxSample_ < 1e-4f && !noteOn_;
}

bool Reed::AddTo(std::span<float> buffer, std::span<float> auxBuffer) {
    maxSample_ = 0.0f;
    UpdateDelayLen();
    for (auto& s : buffer) {
        s += ProcessSingle();
    }
    return maxSample_ < 1e-4f && !noteOn_;
}

bool Reed::IsPlaying(uint8_t note) {
    return note_ == note;
}

bool Reed::CanPlay(uint8_t note) {
    return maxSample_ < 1e-4f || note_ == note;
}

float Reed::ProcessSingle() {
    auto env = envelop_.Process(sustain_);
    auto noise = noise_.Next01() * noiseGain_;
    auto air = (airGain_ + tremoloAmount_ + noise) * env;
    air *= env;
    air /= 2;
    auto pipe = pipe_->GetLast() * realDecay_;
    float delta = air - pipe;
    debugValue_ = delta;
    float injet = air - ReedReflection2(delta) * delta;
    injet = lossHP_.Process(injet);
    auto out = lossLP_.Process(injet);
    // out = tunningFilter_.Process(out);
    pipe_->Push(-out);
    debugValueOutputWave_ = out;
    maxSample_ = std::max(maxSample_, std::abs(out));
    return out;
}

void Reed::NoteOn(uint8_t channel, uint32_t note, float velocity) {
    channel_ = channel;
    note_ = note;
    sustain_ = std::lerp(0.8f, 1.0f, velocity);

    auto freq = Note::Midi2Frequency(note + hpFilterOffset_);
    lossHP_.SetCutoffHPF(freq);
    freq = Note::Midi2Frequency(note + lpFilterOffset_);
    lossLP_.SetCutOffFreq(freq);
    CalcRealDecay();

    freq = Note::Midi2Frequency(note);
    float filterLen = 0.0f;
    filterLen += lossLP_.GetPhaseDelay(freq);
    filterLen += lossHP_.GetPhaseDelay(freq);
    auto secs = 1.0f / freq;
    auto len = secs * sampleRate_ - filterLen;
    waveguideLoopLen_ = len;
    float pitchBendMaxFreq = Note::Midi2Frequency(note + 2);
    float pitchBendSecs = 1.0f / pitchBendMaxFreq;
    float pitchBendLen = pitchBendSecs * sampleRate_ - filterLen;
    pitchBendLenDelta_ = len - pitchBendLen;
    pitchBendLenDelta_ = std::max(pitchBendLenDelta_, 0.0f);
    noteOn_ = true;

    vibrateDelay_.Set(0);
    tremoloDelay_.Set(0);
}

void Reed::NoteOff() {
    sustain_ = 0;
    noteOn_ = false;
}

void Reed::Panic() {
}

void Reed::SetNoiseGain(float gain) {
    noiseGain_ = gain;
}

void Reed::SetLossGain(float gain) {
    lossGain_ = gain;
    realDecay_ = lossGain_ / filterLossGain_;
}

void Reed::SetLossLP(float pitch) {
    lpFilterOffset_ = pitch;
    auto freq = Note::Midi2Frequency(note_ + lpFilterOffset_);
    lossLP_.SetCutOffFreq(freq);
    CalcRealDecay();
}

void Reed::SetLossHP(float pitch) {
    hpFilterOffset_ = pitch;
}

void Reed::SetAttack(float ms) {
    envelop_.SetAttackTime(ms);
}

void Reed::SetRelease(float ms) {
    envelop_.SetReleaseTime(ms);
}

void Reed::AllocDelay(Reed& reed) {
    reed.SetDelayLineRef(DelayAllocator::GetDelayLine());
}

void Reed::FreeDelay(Reed& reed) {
    DelayAllocator::ReleaseDelayLine(reed.pipe_);
    reed.pipe_ = nullptr;
}

float Reed::ReedReflection2(float delta) {
    if (delta < inhalingOffset_) {
        return 0.02;
    }
    else if (delta > activeOffset_) {
        return 0.98;
    }
    else {
        auto normalX = (delta - inhalingOffset_) * slope_;
        auto v = std::tanh((normalX * 2 - 1) * blend_) * scaleFix_ * 0.5f + 0.5f;
        return utli::ClampUncheck(v, 0.02, 0.98);
    }
}

void Reed::CalcRealDecay() {
    auto lpFreq = lossLP_.GetFreq();
    auto hpFreq = lossHP_.GetFreq();
    auto maxFreq = std::sqrt(lpFreq * hpFreq);
    auto omega = maxFreq / sampleRate_ * Note::twopi;
    filterLossGain_ = lossLP_.GetMagPowerResponce(omega) * lossHP_.GetMagPowerResponce(omega);
    filterLossGain_ = std::sqrt(filterLossGain_);
    realDecay_ = lossGain_ / filterLossGain_;
}

void Reed::UpdateDelayLen() {
    float pitchBendAmount = 0.0f;
    if (SynthParams.reed.autoVibrate.Get()) {
        // use auto vibrate
        vibrateOscPhaseInc_ = SynthParams.reed.vibrateRate.Get() / (sampleRate_ / 480);
        vibrateOscPhase_ += vibrateOscPhaseInc_;
        if (vibrateOscPhase_ > 1.0f) {
            vibrateOscPhase_ -= 1.0f;
        }
        float triangle = 4.0f * std::abs(vibrateOscPhase_ - 0.5f) - 1.0f;
        pitchBendAmount = triangle * SynthParams.reed.vibrateDepth.Get();
    }
    else {
        // TODO: use mpe pitchbend
        pitchBendAmount = 0.0f;
    }
    pitchBendAmount *= vibrateDelay_.Process(1);
    float vibrateLen = waveguideLoopLen_ + pitchBendAmount * pitchBendLenDelta_;
    int32_t idelay = tunningFilter_.SetDelay(vibrateLen);
    pipe_->SetDelay(idelay);

    // tremolo process
    if (SynthParams.reed.autoTremolo.Get()) {
        // use auto tremolo
        tremoloOscPhaseInc_ = SynthParams.reed.tremoloRate.Get() / (sampleRate_ / 480);
        tremoloOscPhase_ += tremoloOscPhaseInc_;
        if (tremoloOscPhase_ > 1.0f) {
            tremoloOscPhase_ -= 1.0f;
        }
        float sin = std::sin(tremoloOscPhase_ * std::numbers::pi_v<float> * 2.0f);
        tremoloAmount_ = sin;
    }
    else {
        tremoloAmount_ = 0.0f;
    }
    tremoloAmount_ *= tremoloDelay_.Process(1);
    tremoloAmount_ *= SynthParams.reed.tremoloDepth.Get();
}

void Reed::SetLossFaster(bool faster) {
    if (faster) {
        lossLP_.SetLoopFilterType(Lowpass::LoopFilterType::IIR_LPF2);
    }
    else {
        lossLP_.SetLoopFilterType(Lowpass::LoopFilterType::IIR_LPF1);
    }
    CalcRealDecay();
}

}