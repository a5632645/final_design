#pragma once
#include <span>
#include <array>
#include <cstdint>
#include <cmath>
#include "params.hpp"

namespace dsp {

class Body {
public:
    static constexpr uint32_t kFFTSize = 2048;
    static constexpr uint32_t kBlockSize = 1024;
    static constexpr uint32_t kNumBins = kFFTSize / 2 + 1;

    static constexpr uint32_t kNumBodys = 9;

    static constexpr const char* kBodyNames[kNumBodys] {
        "班卓", "Calcani", "吉他", "Klotz", "Langhof", "班卓2", "Dobro", "提琴", "古筝"
    };
    inline static constexpr const char* GetName(int32_t idx) {
        return kBodyNames[idx];
    }

    struct Frame {
        std::array<float, kNumBins> real_{};
        std::array<float, kNumBins> imag_{};
    };
    struct GainFrame {
        std::array<float, kNumBins> real_{};
        std::array<float, kNumBins> imag_{};
    };

    void Init(float sampleRate);
    void Process(std::span<float> buffer, std::span<float> auxBuffer);

    void SetEnabled(bool enabled) { processing_ = enabled; }
    void SetBodyType(BodyEnum type);
    void SetWetGain(float wet) { WetGain_ = std::pow(10.0f, wet / 20.0f); }
    void SetStretch(float stretch);
private:
    void DoBodyFFT(const float* irPtr);

    uint32_t numInput_{};
    uint32_t writeEnd_{};
    uint32_t writeAddBegin_{};
    bool processing_{};
    float WetGain_{};
    float stretch_{ 1.0f };
    BodyEnum body_;
};

}