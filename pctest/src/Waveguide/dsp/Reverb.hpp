#pragma once
#include <array>
#include <span>
#include <cstdint>
#include "Noise.hpp"
#include "DelayLine.hpp"
#include "OnePoleFilter.hpp"
#include "CombAllpass.hpp"

namespace dsp {

class Reverb {
public:
    static constexpr uint32_t kFIRSize = 2048;
    static constexpr uint32_t kDelayMask = kFIRSize - 1;
    static constexpr uint32_t kTapSize = kFIRSize / 32;

    void Init(uint32_t sampleRate);
    void Process(std::span<float> buffer, std::span<float> auxBuffer);
    
    void SetQuadOscRate(float freq);
    void SetLowpassFreq(float freq);
    void SetDecayTime(float ms);
    void NewVelvetNoise(uint32_t interval);
    void SetEarlyReflectionSize(float size);
    void SetSize(float size);
    void SetModulationDepth(float depth);
    void SetDryWet(float drywet);
    void SetChrousRate(float rate);
    void SetChrousDepth(float depth) { chrousDepth_ = depth; }
private:
    uint32_t sampleRate_ = 0;
    
    // velvet 1
    std::array<float, kFIRSize> delay1_{};
    uint32_t delay1WritePos_ = 0;
    std::array<uint32_t, kTapSize> delay1PosTaps_{};
    uint32_t numDelay1PosTaps_ = 0;
    std::array<uint32_t, kTapSize> delay1NegTaps_{};
    uint32_t numDelay1NegTaps_ = 0;
    std::array<uint32_t, kTapSize> delay2PosTaps_{};
    uint32_t numDelay2PosTaps_ = 0;
    std::array<uint32_t, kTapSize> delay2NegTaps_{};
    uint32_t numDelay2NegTaps_ = 0;
    float gain1_{};
    float gain2_{};

    // velvet 2
    std::array<float, kFIRSize> delay2_{};
    std::array<float, kFIRSize> delay3_{};
    uint32_t delay1WritePos2_ = 0;
    uint32_t delay1WritePos3_ = 0;
    std::array<uint32_t, kTapSize> delay1PosTaps2_{};
    uint32_t numDelay1PosTaps2_ = 0;
    std::array<uint32_t, kTapSize> delay1NegTaps2_{};
    uint32_t numDelay1NegTaps2_ = 0;
    std::array<uint32_t, kTapSize> delay2PosTaps2_{};
    uint32_t numDelay2PosTaps2_ = 0;
    std::array<uint32_t, kTapSize> delay2NegTaps2_{};
    uint32_t numDelay2NegTaps2_ = 0;
    float gain12_{};
    float gain22_{};
    
    uint32_t earlyReflectionSize_ = 0;
    uint32_t velvetInterval_ = 0;

    Noise noise_;
    CombAllpass combAllpass1_;
    CombAllpass combAllpass2_;
    CombAllpass combAllpass3_;
    CombAllpass combAllpass4_;
    CombAllpass combAllpass5_;
    CombAllpass combAllpass6_;
    CombAllpass combAllpass7_;
    CombAllpass combAllpass8_;
    float decay1_{};
    float decay2_{};
    float decay3_{};
    float decay4_{};
    float decay5_{};
    float decay6_{};
    float decay7_{};
    float decay8_{};
    float chrousLatch1_{};
    float chrousLatch2_{};
    float chrousLatch3_{};
    float chrousLatch4_{};
    float chrousLatch5_{};
    float chrousLatch6_{};
    float chrousLatch7_{};
    float chrousLatch8_{};
    float latchAlpha_{};
    float dryWet_{};
    float decayMs_{};

    float quadOscV_{};
    float quadOscU_{ 1.0f };
    float k1_{};
    float k2_{};

    float chrousDepth_{};
    float depth_{};
    float delay1Len_{};
    float delay2Len_{};
    float delay3Len_{};
    float delay4Len_{};
    float delay5Len_{};
    float delay6Len_{};
    float delay7Len_{};
    float delay8Len_{};
    OnePoleFilter lowpass1_;
    OnePoleFilter lowpass2_;
    OnePoleFilter lowpass3_;
    OnePoleFilter lowpass4_;
    OnePoleFilter lowpass5_;
    OnePoleFilter lowpass6_;
    OnePoleFilter lowpass7_;
    OnePoleFilter lowpass8_;
};

} // namespace dsp
