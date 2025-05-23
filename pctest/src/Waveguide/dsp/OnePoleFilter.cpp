#include "OnePoleFilter.hpp"
#include <cmath>
#include <complex>

namespace dsp {

void OnePoleFilter::Init(float sampleRate) {
    sampleRate_ = sampleRate;
}

void OnePoleFilter::SetCutoffLPF(float freq) {
    freq_ = freq;

    freq = std::min(freq, GetMaxLowpassFreq());
    float omega = 2 * std::numbers::pi_v<float> * freq / sampleRate_;
    auto k = std::tan(omega / 2);
    b0_ = k / (1 + k);
    b1_ = b0_;
    a1_ = (k - 1) / (k + 1);
    type_ = Type::kLowpass;
}

void OnePoleFilter::SetCutoffHPF(float freq) {
    freq_ = freq;
    float omega = 2 * std::numbers::pi_v<float> * freq / sampleRate_;
    auto k = std::tan(omega / 2);
    b0_ = 1 / (1 + k);
    b1_ = -b0_;
    a1_ = (k - 1) / (k + 1);
    type_ = Type::kHighpass;
}

float OnePoleFilter::GetMagPowerResponce(float omega) {
    auto cosv = std::cos(omega);
    auto up = b1_ * b1_ + b0_ * b0_ + 2 * b0_ * b1_ * cosv;
    auto down = 1 + a1_ * a1_ + 2 * a1_ * cosv;
    return up / down;
}

float OnePoleFilter::GetMaxLowpassFreq() const {
    return 20000.0f;
}

float OnePoleFilter::GetPhaseDelay(float freq) const {
    auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
    auto z = std::polar(1.0f, omega);
    auto up = b0_ * z + b1_;
    auto down = z + a1_;
    auto response = up / down;
    auto phase = std::arg(response);
    // first order filter always in -pi ~pi, no need unwarp
    return -phase / omega;
}

void OnePoleFilter::CopyCoeff(const OnePoleFilter& other) {
    freq_ = other.freq_;
    b0_ = other.b0_;
    b1_ = other.b1_;
    a1_ = other.a1_;
    type_ = other.type_;
}
}