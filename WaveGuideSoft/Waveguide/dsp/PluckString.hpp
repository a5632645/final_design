#pragma once
#include <span>
#include "DelayLine.hpp"
#include "OnePoleFilter.hpp"
#include "ThrianDispersion.hpp"
#include "Noise.hpp"
#include "TuningFilter.hpp"
#include "DCBlocker.hpp"
#include "Lowpass.hpp"

namespace dsp {

class PluckString {
public:
    void Init(float sampleRate);
    void NoteOn(uint8_t channel, uint8_t noteNumber, float velocity);
    void NoteOff();
    bool Process(std::span<float> buffer, std::span<float> auxBuffer);
    float ProcessSingle();
    bool AddTo(std::span<float> buffer, std::span<float> auxBuffer);
    bool CanPlay(uint8_t note);
    bool IsPlaying(uint8_t note);
    void SetDelayLineRef(DelayLine* str1, DelayLine* /*str2*/) { delay_ = str1; }
    void Panic();

    // setter
    void SetDecay(float decay);
    void SetLossLPLow(float pitch);
    void SetDispersion(float disp);
    void SetPluckPosition(float pos);
    void SetColor(float color);
    void SetLossFaster(bool faster);
    void SetExciterFaster(bool faster);
    void SetDetune(float pitch) { detunePitch_ = pitch; }

    // delay allocate
    static void AllocDelay(PluckString& string);
    static void FreeDelay(PluckString& string);
private:
    void UpdateParam();
    float GetLossLP(int32_t note);
    float GetExciLP(int32_t note);

    uint8_t channel_{};
    ThrianDispersion dispersion_;
    DelayLine* delay_;
    Lowpass lossLP_;
    TunningFilter tunningFilter_;
    DCBlocker995 dcBlocker_;
    Noise noise_;
    Noise noise2_;
    uint32_t noiseSeed_{};
    uint8_t note_{};
    float decay_{};
    float pluckPosition_{};
    float sampleRate_{};
    float dispersionLenRatio_{};
    float delayLen_{};
    float waveguideLoopLen_{};
    float pitchBendLenDelta_{};
    float pluseLen_{};
    float noisePhase_{};
    float noisePhase2_{};
    bool  generateClick_{ false };
    float color_{};
    float maxSample_{};
    float detunePitch_{};
    Lowpass exciterFilter_;
    float decayTime_{};
};

} // namespace dsp
