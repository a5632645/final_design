#include "Lowpass.hpp"
#include <cmath>
#include <numbers>
#include <complex>

namespace dsp {

void Lowpass::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    SetLPF1(1000.0f);
}

float Lowpass::Process(float x) {
    auto t = x - a1_ * latch1_ - a2_ * latch2_;
    auto y = t * b0_ + b1_ * latch1_ + b2_ * latch2_;
    latch2_ = latch1_;
    latch1_ = t;

    // t = y - a21_ * latch21_ - a22_ * latch22_;
    // y = t * b20_ + b21_ * latch21_ + b22_ * latch22_;
    // latch22_ = latch21_;
    // latch21_ = t;
    return y;
}

void Lowpass::SetCutOffFreq(float freq) {
    if (freq > GetMaxFreq()) {
        freq = GetMaxFreq();
    }
    freq_ = freq;
    switch (loopFilterType_) {
    case LoopFilterType::IIR_LPF1:
        SetLPF1(freq);
        break;
    case LoopFilterType::IIR_LPF2:
        SetLPF2(freq);
        break;
    // case LoopFilterType::IIR_LPF3:
    //     SetLPF3(freq);
    //     break;
    // case LoopFilterType::IIR_LPF4:
    //     SetLPF4(freq);
    //     break;
    }
}

float Lowpass::GetMaxFreq() const {
    return 20000.0f;
}

void Lowpass::SetLPF1(float freq) {
    float omega = 2 * std::numbers::pi_v<float> * freq / sampleRate_;
    auto k = std::tan(omega / 2);
    b0_ = k / (1 + k);
    b1_ = b0_;
    b2_ = 0;
    a1_ = (k - 1) / (k + 1);
    a2_ = 0;

    // b20_ = 1;
    // b21_ = 0;
    // b22_ = 0;
    // a21_ = 0;
    // a22_ = 0;
}

void Lowpass::SetLPF2(float freq) {
    auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
    auto k = std::tan(omega / 2);
    constexpr auto Q = 1.0f / std::numbers::sqrt2_v<float>;
    auto down = k * k * Q + k + Q;
    b0_ = k * k * Q / down;
    b1_ = 2 * b0_;
    b2_ = b0_;
    a1_ = 2 * Q * (k * k - 1) / down;
    a2_ = (k * k * Q - k + Q) / down;

    // b20_ = 1;
    // b21_ = 0;
    // b22_ = 0;
    // a21_ = 0;
    // a22_ = 0;
}

// void Lowpass::SetLPF3(float freq) {
//     auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
//     auto k = std::tan(omega / 2);
//     constexpr auto Q = 1.0f;
//     auto down = k * k * Q + k + Q;
//     b0_ = k * k * Q / down;
//     b1_ = 2 * b0_;
//     b2_ = b0_;
//     a1_ = 2 * Q * (k * k - 1) / down;
//     a2_ = (k * k * Q - k + Q) / down;

//     b20_ = k / (1 + k);
//     b21_ = b20_;
//     b22_ = 0;
//     a21_ = (k - 1) / (k + 1);
//     a22_ = 0;
// }

// void Lowpass::SetLPF4(float freq) {
//     auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
//     auto k = std::tan(omega / 2);
//     auto Q = 0.54119610f;
//     auto down = k * k * Q + k + Q;
//     b0_ = k * k * Q / down;
//     b1_ = 2 * b0_;
//     b2_ = b0_;
//     a1_ = 2 * Q * (k * k - 1) / down;
//     a2_ = (k * k * Q - k + Q) / down;

//     Q = 1.3065630f;
//     down = k * k * Q + k + Q;
//     b20_ = k * k * Q / down;
//     b21_ = 2 * b20_;
//     b22_ = b20_;
//     a21_ = 2 * Q * (k * k - 1) / down;
//     a22_ = (k * k * Q - k + Q) / down;
// }

float Lowpass::GetPhaseDelay(float freq) const {
    auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
    auto z = std::polar(1.0f, omega);
    auto up = b0_ * z * z + b1_ * z + b2_;
    auto down = z * z + a1_ * z + a2_;
    auto response = up / down;
    auto phase = std::arg(response);
    if (phase > 0) phase -= 2 * std::numbers::pi_v<float>;
    return -phase / omega;
}

void Lowpass::Panic() {
    latch1_ = 0.0f;
    latch2_ = 0.0f;
}

void Lowpass::SetLoopFilterType(LoopFilterType type) {
    if (loopFilterType_ != type) {
        loopFilterType_ = type;
        SetCutOffFreq(freq_);
    }
}

float Lowpass::GetMagPowerResponce(float omega) const {
    auto z = std::polar(1.0f, omega);
    auto up = b2_ * z * z + b1_ * z + 1.0f;
    auto down = z * z + b1_ * z + b2_;
    auto response = up / down;
    return std::norm(response);
}

void Lowpass::CopyCoeff(const Lowpass& other) {
    loopFilterType_ = other.loopFilterType_;
    freq_ = other.freq_;
    a1_ = other.a1_;
    a2_ = other.a2_;
    b0_ = other.b0_;
    b1_ = other.b1_;
    b2_ = other.b2_;
}
}