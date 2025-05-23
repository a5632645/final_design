#include "PluckString.hpp"
#include "Note.hpp"
#include "DelayAllocator.hpp"
#include <cassert>
#include <algorithm>
#include "utli/Map.hpp"
#include "utli/Clamp.hpp"
#include "utli/Lerp.hpp"
#include "params.hpp"

namespace dsp {

static Noise globalNoise_;

void PluckString::Init(float sampleRate) {
    dispersion_.Init(sampleRate);
    delay_->Init(sampleRate);
    lossLP_.Init(sampleRate);
    exciterFilter_.Init(sampleRate);
    tunningFilter_.Init(sampleRate);
    lossLP_.SetLoopFilterType(Lowpass::LoopFilterType::IIR_LPF2);
    sampleRate_ = sampleRate;
}


void PluckString::NoteOn(uint8_t channel, uint8_t noteNumber, float velocity) {
    channel_ = channel;
    note_ = noteNumber;
    auto freq = Note::Midi2Frequency(noteNumber);
    auto secs = 1.0f / freq;

    float lossLPSt = GetLossLP(noteNumber);
    lossLP_.SetCutOffFreq(Note::Midi2Frequency(lossLPSt));
    auto len = secs * sampleRate_;
    auto dispLen = dispersionLenRatio_ * len;
    dispersion_.SetGroupDelay(dispLen);
    float dispRealLen = dispersion_.GetPhaseDelay(freq);
    len -= dispRealLen;
    len -= lossLP_.GetPhaseDelay(freq);
    float delayLen = tunningFilter_.SetDelay(len);
    delay_->SetDelay(delayLen);

    generateClick_ = true;
    delayLen_ = len;
    pluseLen_ = delayLen_ * pluckPosition_;
    noisePhase2_ = 0;
    noisePhase_ = 0;
    noiseSeed_ = globalNoise_.NextUInt();
    noise_.SetSeed(noiseSeed_);
    noise2_.SetSeed(noiseSeed_);

    // set filter and calc decay
    // float lpPitch = utli::Lerp(SynthParams.string.exciLow.Get(), SynthParams.string.exciHigh.Get(), note_ / 127.0f);
    exciterFilter_.SetCutOffFreq(Note::Midi2Frequency(GetExciLP(noteNumber)));
    SetDecay(decayTime_);
}

void PluckString::NoteOff() {
    // do nothing
}

bool PluckString::Process(std::span<float> buffer, std::span<float> auxBuffer) {
    maxSample_ = 0.0f;
    UpdateParam();

    for (int i = 0; auto& s : buffer) {
        s = ProcessSingle();
    }
    return maxSample_ < 1e-4f;
}

float PluckString::ProcessSingle() {
#if 0
    if (generateClick_) {
        generateClick_ = false;
        return 1;
    }
    else {
        return 0;
    }
#endif

    float in = 0.0f;
    if (generateClick_) {
        float dc = 0.0f;
        float noise = 0.0f;
        if (noisePhase_ < pluseLen_) {
            dc = 0.5f;
        }
        else if (noisePhase_ < delayLen_) {
            dc = -0.5f;
        }
        if (noisePhase_ < delayLen_) {
            noise += noise_.Next();
        }
        if (noisePhase2_ - pluseLen_ > 0.0f) {
            noise -= noise2_.Next();
        }
        if (noisePhase2_ - pluseLen_ > delayLen_) {
            generateClick_ = false;
        }
        noisePhase_ += 1.0f;
        noisePhase2_ += 1.0f;
        in = utli::Lerp(dc, noise, color_);
    }
    in = dcBlocker_.Process(in);
    in = exciterFilter_.Process(in) / 2;

    auto a = delay_->GetLast() + in;
    auto out = a;
    a = lossLP_.Process(a);
    a = dispersion_.Process(a);
    a = tunningFilter_.Process(a);
    a = utli::Clamp(a, -4.0f, 4.0f);
    a *= decay_;
    delay_->Push(a);

    maxSample_ = std::max(maxSample_, std::abs(out));
    return out;
}

bool PluckString::AddTo(std::span<float> buffer, std::span<float> auxBuffer) {
    maxSample_ = 0.0f;
    UpdateParam();

    for (int i = 0; auto& s : buffer) {
        s += ProcessSingle();
    }
    return maxSample_ < 1e-4f;
}

void PluckString::UpdateParam() {
}

bool PluckString::CanPlay(uint8_t note) {
    return maxSample_ < 1e-4f;
}

bool PluckString::IsPlaying(uint8_t note) {
    return note == note_;
}

void PluckString::Panic() {
    // dispersion_.Panic();
    // lossLP_.ClearInteral();
}

void PluckString::SetDecay(float d) {
    decayTime_ = d;
    if (d > 0.0f) {
        auto mul = 1.0f / (static_cast<float>(sampleRate_) * d / 1000.0f);
        auto t = std::pow(10.0f, -(static_cast<float>(delayLen_ * mul)));
        decay_ = std::min(0.9999f, t);
    }
    else if (d < 0.0f) {
        auto mul = 1.0f / (static_cast<float>(sampleRate_) * -d / 1000.0f);
        auto t = -std::pow(10.0f, -(static_cast<float>(delayLen_ * mul)));
        decay_ = std::max(-0.9999f, t);
    }
}

void PluckString::SetLossLPLow(float pitch) {
    // lossLP_.SetCutOffFreq(Note::Midi2Frequency(pitch));
}

void PluckString::SetDispersion(float ratio) {
    dispersionLenRatio_ = ratio;
}

void PluckString::SetPluckPosition(float pos) {
    pluckPosition_ = pos;
}

void PluckString::SetColor(float color) {
    color_ = color;
}

void PluckString::SetLossFaster(bool faster) {
    if (faster) {
        lossLP_.SetLoopFilterType(Lowpass::LoopFilterType::IIR_LPF2);
    }
    else {
        lossLP_.SetLoopFilterType(Lowpass::LoopFilterType::IIR_LPF1);
    }
}

void PluckString::SetExciterFaster(bool faster) {
    if (faster) {
        exciterFilter_.SetLoopFilterType(Lowpass::LoopFilterType::IIR_LPF2);
    }
    else {
        exciterFilter_.SetLoopFilterType(Lowpass::LoopFilterType::IIR_LPF1);
    }
}

void PluckString::AllocDelay(PluckString& string) {
    string.SetDelayLineRef(DelayAllocator::GetDelayLine(), DelayAllocator::GetDelayLine());
}

void PluckString::FreeDelay(PluckString& string) {
    DelayAllocator::ReleaseDelayLine(string.delay_);
}

float PluckString::GetLossLP(int32_t note) {
    auto inLow = SynthParams.string.lossTructionlow.Get();
    auto inHigh = SynthParams.string.lossTructionHigh.Get();
    auto outLow = SynthParams.string.lossOutLow.Get();
    auto outHigh = SynthParams.string.lossOutHigh.Get();
    if (note <= inLow) {
        return outLow;
    }
    else if (note >= inHigh) {
        return outHigh;
    }
    else {
        auto ratio = (note - inLow) / (static_cast<float>(inHigh) - static_cast<float>(inLow));
        auto out = outLow + ratio * (outHigh - outLow);
        return out;
    }
}

float PluckString::GetExciLP(int32_t note) {
    auto inLow = SynthParams.string.exciTructionlow.Get();
    auto inHigh = SynthParams.string.exciTructionHigh.Get();
    auto outLow = SynthParams.string.exciOutLow.Get();
    auto outHigh = SynthParams.string.exciOutHigh.Get();
    if (note <= inLow) {
        return outLow;
    }
    else if (note >= inHigh) {
        return outHigh;
    }
    else {
        auto ratio = (note - inLow) / (static_cast<float>(inHigh) - static_cast<float>(inLow));
        auto out = outLow + ratio * (outHigh - outLow);
        return out;
    }
}
}