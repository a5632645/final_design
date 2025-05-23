#include "SVF.hpp"
#include <cmath>
#include <numbers>

namespace dsp {

void SVF::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    q_ = std::numbers::sqrt2_v<float>;
}

void SVF::SetCutoffFreq(float freq) {
    if (freq > 8000) freq = 8000;
    f_ = 2.0f * std::sin(std::numbers::pi_v<float> * freq / sampleRate_);
}

float SVF::Process(float in) {
    latch2_ = latch2_ + latch1_ * f_;
    lowpass_ = latch2_;
    highpass_ = in - latch2_ - q_ * latch1_;
    latch1_ = highpass_ * f_ + latch1_;
    bandpass_ = latch1_;
    bandreject_ = highpass_ + lowpass_;
    return lowpass_;
}

}