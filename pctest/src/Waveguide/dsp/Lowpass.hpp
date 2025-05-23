#pragma once

namespace dsp {

class Lowpass {
public:
    enum class LoopFilterType {
        IIR_LPF1,
        IIR_LPF2,
        // IIR_LPF3,
        // IIR_LPF4
    };

    void  Init(float sampleRate);
    float Process(float x);
    void  SetCutOffFreq(float freq);
    void  SetLPF1(float freq);
    void  SetLPF2(float freq);
    // void  SetLPF3(float freq);
    // void  SetLPF4(float freq);
    float GetPhaseDelay(float freq) const;
    void  Panic();
    float GetMaxFreq() const;
    void  SetLoopFilterType(LoopFilterType type);
    LoopFilterType GetLoopFilterType() const { return loopFilterType_; }
    float GetFreq() const { return freq_; }
    float GetMagPowerResponce(float omega) const;
    void  CopyCoeff(const Lowpass& other);
private:
    LoopFilterType loopFilterType_{ LoopFilterType::IIR_LPF2 };
    float freq_{};
    float sampleRate_{};
    float latch1_{};
    float latch2_{};
    float a1_{};
    float a2_{};
    float b0_{};
    float b1_{};
    float b2_{};
    // filter 2
    // float latch21_{};
    // float latch22_{};
    // float a21_{};
    // float a22_{};
    // float b20_{};
    // float b21_{};
    // float b22_{};
};

}
