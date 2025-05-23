#pragma once
#include <cstdint>
#include <span>
#include "DelayLine.hpp"
#include "OnePoleFilter.hpp"
#include "ExpSmoother.hpp"
#include "Noise.hpp"
#include "TuningFilter.hpp"
#include "Lowpass.hpp"
#include "ExpSmoother2.hpp"

namespace dsp {

class Reed {
public:
    void Init(float sampleRate);
    bool Process(std::span<float> buffer, std::span<float> auxBuffer);
    bool AddTo(std::span<float> buffer, std::span<float> auxBuffer);
    bool IsPlaying(uint8_t note);
    bool CanPlay(uint8_t note);
    void NoteOn(uint8_t channel, uint32_t note, float velocity);
    void NoteOff();
    void SetDelayLineRef(DelayLine* pipe) { pipe_ = pipe; }
    void Panic();
    float ProcessSingle();
    // setter
    void SetNoiseGain(float gain);
    void SetLossGain(float gain);
    void SetLossLP(float pitch);
    void SetLossHP(float pitch);
    void SetAttack(float ms);
    void SetRelease(float ms);
    void SetAirGain(float gain) { airGain_ = gain; }
    void SetInhalingOffset(float offset) { inhalingOffset_ = offset; slope_ = 1.0f / (activeOffset_ - inhalingOffset_); }
    void SetActiveOffset(float offset) { activeOffset_ = offset; slope_ = 1.0f / (activeOffset_ - inhalingOffset_); }
    void SetBlend(float blend);
    void SetLossFaster(bool faster);
    void SetTremoloAttack(float ms) { tremoloDelay_.SetTime(ms); }
    void SetVibrateAttack(float ms) { vibrateDelay_.SetTime(ms); }
    // delay allocate
    static void AllocDelay(Reed& reed);
    static void FreeDelay(Reed& reed);
    
    float ReedReflection2(float delta);

    float debugValue_{};
    float debugValueOutputWave_{};
private:
    void CalcRealDecay();
    void UpdateDelayLen();

    uint8_t channel_{};
    DelayLine* pipe_;
    OnePoleFilter noiseLP_;
    Lowpass lossLP_;
    OnePoleFilter lossHP_;
    ExpSmoother envelop_;
    Noise noise_;
    TunningFilter tunningFilter_;
    float sustain_{};
    float inhalingOffset_{};
    float activeOffset_{};
    float blend_{};
    float slope_{};
    float scaleFix_{};
    float noiseGain_{};
    float lossGain_{};
    float sampleRate_{};
    float maxSample_{};
    float airGain_{};
    uint8_t note_{};
    bool noteOn_{};
    float realDecay_{};
    float filterLossGain_{};
    float hpFilterOffset_{};
    float lpFilterOffset_{};
    float waveguideLoopLen_{};

    // tremolo
    float tremoloOscPhase_{};
    float tremoloOscPhaseInc_{};
    float tremoloAmount_{};
    ExpSmoother2 tremoloDelay_;
    // vibrate
    float vibrateOscPhase_{};
    float vibrateOscPhaseInc_{};
    float pitchBendLenDelta_{};
    ExpSmoother2 vibrateDelay_;
};

}
