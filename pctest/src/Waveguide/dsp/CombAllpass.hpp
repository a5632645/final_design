#pragma once
#include "DelayLine.hpp"

namespace dsp {

class CombAllpass {
public:
    void  Init(float sampleRate);
    float Process(float in);
    void  ClearInternal();
    void  SetDelay(float delay);
    float GetDelay() const { return delay_.GetDelay(); }
    void  SetAlpha(float a);
    float GetAlpha() const { return alpha_; }
    float GetLast() const { return last_; }
private:
    DelayLine delay_;
    float alpha_{};
    float last_{};
};

class Allpass {
public:
    float Process(float in) {
        auto v = latch_;
        auto t = in - alpha_ * v;
        latch_ = t;
        return v + alpha_ * t;
    }
    void  ClearInternal() {
        latch_ = 0;
    }
    void  SetAlpha(float a) {
        alpha_ = a;
    }
private:
    float alpha_{};
    float latch_{};
};

}