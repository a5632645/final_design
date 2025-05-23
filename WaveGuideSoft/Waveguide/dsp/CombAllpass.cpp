#include "CombAllpass.hpp"

namespace dsp {

void CombAllpass::Init(float sampleRate) {
    delay_.Init(sampleRate);
}

float CombAllpass::Process(float in) {
    auto v = delay_.GetLast();
    auto t = in - alpha_ * v;
    delay_.Push(t);
    last_ = v + alpha_ * t;
    return last_;
}

void CombAllpass::ClearInternal() {
    delay_.ClearInternal();
}

void CombAllpass::SetDelay(float delay) {
    delay_.SetDelay(delay);
}

void CombAllpass::SetAlpha(float a) {
    alpha_ = a;
}

}