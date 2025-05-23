#pragma once
#include <cmath>
#include <numbers>
#include <cstdint>

namespace dsp {

struct Note {
    static constexpr float kA4 = 440.0f;
    static constexpr int32_t kA4Midi = 69;

    static constexpr float kA0 = 27.5f;
    static constexpr int32_t kA0Midi = 21;

    static float Midi2Frequency(float midi) {
        return kA4 * std::exp2((midi - kA4Midi) / 12.0f);
    }

    static float Hz2Sec(float hz) {
        return 1.0f / hz; 
    }

    static float Midi2Sec(float midi) {
        return Hz2Sec(Midi2Frequency(midi)); 
    }

    static constexpr float pi = std::numbers::pi_v<float>;
    static constexpr float twopi = 2.0f * pi;
};

} // namespace dsp
