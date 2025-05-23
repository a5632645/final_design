#include "Bowed.hpp"
#include "Note.hpp"
#include "Util.hpp"
#include "DelayAllocator.hpp"
#include "params.hpp"
#include "MidiManager.hpp"

namespace dsp {

void Bowed::Init(float sampleRate) {
    nutBowDelay_->Init(sampleRate);
    bowBridgeDelay_->Init(sampleRate);
    lossLP_.Init(sampleRate);
    sampleRate_ = sampleRate;
    tunningFilter_.Init(sampleRate);
    speedEnv_.Init(sampleRate);
    noiseLP_.Init(sampleRate);

    tremoloDelay_.Init(sampleRate / 480);
    vibrateDelay_.Init(sampleRate / 480);
    tremoloOscPhase_ = 0.0f;
    vibrateOscPhase_ = 0.0f;
}

float Bowed::ProcessSingle() {
    float env = speedEnv_.Process(sustain_);
    currBowSpeed_ = env;
    float noise = noise_.Next();
    noise = noiseLP_.Process(noise) * noiseAmount_;
    auto bowSpeed = (bowSpeed_ + tremoloAmount_ + noise) * env;

    auto brige = -bowBridgeDelay_->GetLast();
    brige = tunningFilter_.Process(brige);
    auto bow = -nutBowDelay_->GetLast();
    auto deltaV = bowSpeed - (bow + brige);
    deltaDebugValue_ = deltaV;
    auto reflection = BowReflectionTable(deltaV);
    auto vinc = reflection * deltaV;
    nutBowDelay_->Push(vinc + brige);
    auto bowGo = vinc + bow;
    bowGo *= decayGain_;
    float out = lossLP_.Process(bowGo);
    bowBridgeDelay_->Push(out);
    waveOutputDebugValue_ = out;

    maxSample_ = std::max(maxSample_, std::abs(out));
    return out;
}

float Bowed::ProcessSingleNoBow() {
    float brige = -bowBridgeDelay_->GetLast();
    brige = tunningFilter_.Process(brige);
    float bowGo = -nutBowDelay_->Process(brige);
    bowGo *= decayGain_;
    float out = lossLP_.Process(bowGo);
    bowBridgeDelay_->Push(out);
    maxSample_ = std::max(maxSample_, std::abs(out));
    return out;
}

void Bowed::NoteOn(uint8_t channel, uint32_t note, float velocity) {
    channel_ = channel;
    note_ = note;
    sustain_ = velocity;
    noteOned_ = true;
    bowUp_ = true;

    auto lossLPSt = GetLossLP(note);
    auto lossFreq = Note::Midi2Frequency(lossLPSt);
    lossLP_.SetCutOffFreq(lossFreq);

    float freq = Note::Midi2Frequency(note);
    float vibrateFreq = Note::Midi2Frequency(note + 2);
    float filterLen = lossLP_.GetPhaseDelay(freq);
    totalLoopLen_ = sampleRate_ / freq;
    waveguideLoopLen_ = totalLoopLen_ - filterLen;
    float vibrateLen = sampleRate_ / vibrateFreq - filterLen;
    pitchBendLenDelta_ = totalLoopLen_ - filterLen - vibrateLen;

    vibrateDelay_.Set(0);
    tremoloDelay_.Set(0);

    float d = SynthParams.bow.decay.Get();
    auto mul = 1.0f / (static_cast<float>(sampleRate_) * d / 1000.0f);
    auto t = std::pow(10.0f, -(static_cast<float>(totalLoopLen_ * mul)));
    decayGain_ = std::min(0.9999f, t);
}

void Bowed::NoteOff() {
    noteOned_ = false;
    sustain_ = 0;
}

void Bowed::Panic() {
}

bool Bowed::Process(std::span<float> buffer, std::span<float> auxBuffer) {
    UpdateParam();
    maxSample_ = 0.0f;

    if (bowUp_) {
        for (auto& s : buffer) {
            s = ProcessSingle();
        }
    }
    else {
        for (auto& s : buffer) {
            s = ProcessSingleNoBow();
        }
    }

    if (currBowSpeed_ < 1e-3f && !noteOned_) {
        bowUp_ = false;
    }

    return maxSample_ < 1e-3f && !bowUp_ && !noteOned_;
}

bool Bowed::AddTo(std::span<float> buffer, std::span<float> auxBuffer) {
    UpdateParam();
    maxSample_ = 0.0f;

    if (bowUp_) {
        for (auto& s : buffer) {
            s += ProcessSingle();
        }
    }
    else {
        for (auto& s : buffer) {
            s += ProcessSingleNoBow();
        }
    }

    if (currBowSpeed_ < 1e-3f && !noteOned_) {
        bowUp_ = false;
    }

    return maxSample_ < 1e-3f && !bowUp_ && !noteOned_;
}

void Bowed::UpdateParam() {
    noiseAmount_ = SynthParams.bow.noise.Get();

    float pitchBendAmount = 0.0f;
    if (SynthParams.bow.autoVibrate.Get()) {
        // use auto vibrate
        vibrateOscPhaseInc_ = SynthParams.bow.vibrateRate.Get() / (sampleRate_ / 480);
        vibrateOscPhase_ += vibrateOscPhaseInc_;
        if (vibrateOscPhase_ > 1.0f) {
            vibrateOscPhase_ -= 1.0f;
        }
        float triangle = 4.0f * std::abs(vibrateOscPhase_ - 0.5f) - 1.0f;
        pitchBendAmount = triangle * SynthParams.bow.vibrateDepth.Get();
    }
    else {
        // TODO: use mpe pitchbend
        pitchBendAmount = 0.0f;
    }
    pitchBendAmount *= vibrateDelay_.Process(1);
    float vibrateLen = waveguideLoopLen_ + pitchBendAmount * pitchBendLenDelta_;
    float nutLen = vibrateLen * bowPosition_;
    float bowLen = vibrateLen - nutLen;
    float fractionNut = nutLen - std::floor(nutLen);
    bowLen += fractionNut;
    nutLen = std::floor(nutLen);
    nutBowDelay_->SetDelay(nutLen);
    int32_t iBowDelay = tunningFilter_.SetDelay(bowLen);
    bowBridgeDelay_->SetDelay(iBowDelay);

    // tremolo process
    if (SynthParams.bow.autoTremolo.Get()) {
        // use auto tremolo
        tremoloOscPhaseInc_ = SynthParams.bow.tremoloRate.Get() / (sampleRate_ / 480);
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
    tremoloAmount_ *= SynthParams.bow.tremoloDepth.Get();
}

int32_t Bowed::GetLossLP(int32_t note) {
    auto inLow = SynthParams.bow.lossTructionlow.Get();
    auto inHigh = SynthParams.bow.lossTructionHigh.Get();
    auto outLow = SynthParams.bow.lossOutLow.Get();
    auto outHigh = SynthParams.bow.lossOutHigh.Get();
    if (note <= inLow) {
        return outLow;
    }
    else if (note >= inHigh) {
        return outHigh;
    }
    else {
        auto ratio = (note - inLow) / (static_cast<float>(inHigh) - static_cast<float>(inLow));
        auto out = outLow + ratio * (outHigh - outLow);
        return static_cast<int32_t>(out);
    }
}

bool Bowed::IsPlaying(uint8_t note) {
    return note_ == note;
}

bool Bowed::CanPlay(uint8_t note) {
    return maxSample_ < 1e-3f || note_ == note;
}

float Bowed::BowReflectionTable(float delata) {
    delata += bowTableOffset_;
    delata *= bowTableSlope_;
    delata = std::abs(delata) + 0.75;
    delata = std::pow(delata, -4);
    delata = utli::ClampUncheck(delata, bowTableMin_, bowTableMax_);
    return delata;
}

void Bowed::SetBowPosition(float pos) {
    bowPosition_ = pos;
}

void Bowed::SetLossLPF(float pitch) {
    // auto freq = Note::Midi2Frequency(pitch);
    // lossLP_.SetCutOffFreq(freq);
}

void Bowed::SetLossFaster(bool faster) {
    if (faster) {
        lossLP_.SetLoopFilterType(Lowpass::LoopFilterType::IIR_LPF2);
    }
    else {
        lossLP_.SetLoopFilterType(Lowpass::LoopFilterType::IIR_LPF1);
    }
}

void Bowed::SetNoiseLP(float st) {
    auto freq = Note::Midi2Frequency(st);
    noiseLP_.SetCutoffLPF(freq);
}

void Bowed::AllocDelay(Bowed &bowed)
{
    bowed.SetDelayLineRef(DelayAllocator::GetDelayLine(), DelayAllocator::GetDelayLine());
}

void Bowed::FreeDelay(Bowed& bowed) {
    DelayAllocator::ReleaseDelayLine(bowed.nutBowDelay_);
    DelayAllocator::ReleaseDelayLine(bowed.bowBridgeDelay_);
}

}