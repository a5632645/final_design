#pragma once

namespace dsp::utli {

inline constexpr float ClampUncheck(float value, float min, float max) {
    return value < min ? min : value > max ? max : value;
}

inline constexpr float LerpUncheck(float in0, float in2, float x) {
    return in0 + x * (in2 - in0);
}

}