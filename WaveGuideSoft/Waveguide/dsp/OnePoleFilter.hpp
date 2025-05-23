#pragma once
#include <cmath>
#include <numbers>

namespace dsp {

class OnePoleFilter {
public:
    enum class Type {
        kLowpass,
        kHighpass
    };

    void  Init(float sampleRate);
    void  SetCutoffLPF(float freq);
    void  SetCutoffHPF(float freq);
    float GetMagPowerResponce(float omega);
    float GetMaxLowpassFreq() const;
    float GetFreq() const { return freq_; }
    float GetPhaseDelay(float freq) const;
    void  CopyCoeff(const OnePoleFilter& other);

    void ClearInteral() {
        latch1_ = 0;
    }

    float Process(float in) {
        auto t = in - a1_ * latch1_;
        auto y = t * b0_ + b1_ * latch1_;
        latch1_ = t;
        return y;
    }
private:
    float sampleRate_ = 0;
    float freq_ = 0;
    float b0_ = 0;
    float b1_ = 0;
    float a1_ = 0;
    float latch1_ = 0;
    Type type_ = Type::kLowpass;
};

}