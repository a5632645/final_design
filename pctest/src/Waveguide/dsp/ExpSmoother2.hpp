#pragma once
#include <cmath>

namespace dsp{

class ExpSmoother2 {
public:
    void Init(float sampleRate) {
        sampleRate_ = sampleRate;
    }

    float Process(float in) {
        if (in > latch_) {
            latch_ = latch_ * a_ + (1 - a_) * in;
        }
        else {
            latch_ = latch_ * a_ + (1 - a_) * in;
        }
        return latch_;
    }

    void SetTime(float ms) {
        ms_ = ms;
        a_ = std::exp(-1.0f / (sampleRate_ * ms / 1000.0f));
    }

    float GetTime() const { return ms_; }

    void CopyCoeff(const ExpSmoother2& other) {
        a_ = other.a_;
        ms_ = other.ms_;
    }

    void Set(float v) {
        latch_ = v;
    }

private:
    float sampleRate_ = 0;
    float latch_ = 0;
    float a_ = 0;
    float ms_ = 0;
};

}