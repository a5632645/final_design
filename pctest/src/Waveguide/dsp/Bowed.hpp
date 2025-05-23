#pragma once
#include <cstdint>
#include <span>
#include "DelayLine.hpp"
#include "OnePoleFilter.hpp"
#include "DCBlocker.hpp"
#include "Lowpass.hpp"
#include "TuningFilter.hpp"
#include "ExpSmoother2.hpp"
#include "Noise.hpp"
#include "ExpSmoother.hpp"

namespace dsp {

class Bowed {
public:
    void  Init(float sampleRate);
    float ProcessSingle();
    float ProcessSingleNoBow();
    void  NoteOn(uint8_t channel, uint32_t note, float velocity);
    void  NoteOff();
    void  SetDelayLineRef(DelayLine* nut, DelayLine* bridge) { nutBowDelay_ = nut; bowBridgeDelay_ = bridge; }
    void  Panic();
    bool  Process(std::span<float> buffer, std::span<float> auxBuffer);
    bool  AddTo(std::span<float> buffer, std::span<float> auxBuffer);
    bool  IsPlaying(uint8_t note);
    bool  CanPlay(uint8_t note);
    // setters
    void SetBowPosition(float pos);
    void SetBowSpeed(float speed) { bowSpeed_ = speed; }
    void SetBowTableSlope(float slope) { bowTableSlope_ = 5 - 4 * slope; }
    void SetBowTableOffset(float offset) { bowTableOffset_ = offset; }
    void SetBowTableMin(float min) { bowTableMin_ = min; }
    void SetBowTableMax(float max) { bowTableMax_ = max; }
    void SetLossGain(float gain) {}
    void SetLossLPF(float pitch);
    void SetLossFaster(bool faster);
    void SetTremoloAttack(float ms) { tremoloDelay_.SetTime(ms); }
    void SetVibrateAttack(float ms) { vibrateDelay_.SetTime(ms); }
    void SetNoiseLP(float st);
    void SetAttack(float ms) { speedEnv_.SetAttackTime(ms); }
    void SetRelease(float ms) { speedEnv_.SetReleaseTime(ms); }
    // delay allocate
    static void AllocDelay(Bowed& bowed);
    static void FreeDelay(Bowed& bowed);

    float BowReflectionTable(float delta);
    float deltaDebugValue_{};
    float waveOutputDebugValue_{};
private:
    void UpdateParam();
    int32_t GetLossLP(int32_t note);

    uint8_t channel_{};
    DelayLine* nutBowDelay_;
    DelayLine* bowBridgeDelay_;
    Lowpass lossLP_;
    TunningFilter tunningFilter_;
    Noise noise_;
    OnePoleFilter noiseLP_;
    ExpSmoother speedEnv_;
    float sampleRate_{};
    float bowPosition_{};
    float totalLoopLen_{};
    float bowSpeed_{};
    float currBowSpeed_{};
    float bowTableSlope_{};
    float bowTableOffset_{};
    float bowTableMin_{};
    float bowTableMax_{};
    float sustain_{};
    uint8_t note_{};
    float maxSample_{};
    bool noteOned_{};
    bool bowUp_{};
    float noiseAmount_{};
    float decayGain_{};

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