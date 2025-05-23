#include "TuningFilter.hpp"
#include <cmath>

namespace dsp {

float TunningFilter::Process(float in) {
    auto v = latch_;
    auto t = in - alpha_ * v;
    latch_ = t;
    return v + alpha_ * t;
}

int32_t TunningFilter::SetDelay(float delay) {
    // thiran delay limit to 0.5 ~ 1.5
    if (delay < 0.5f) {
        alpha_ = 0.0f; // equal to one delay
        return 0;
    }
    else {
        float intergalPart = std::floor(delay);
        float fractionalPart = delay - intergalPart;
        if (fractionalPart < 0.5f) {
            fractionalPart += 1.0f;
            intergalPart -= 1.0f;
        }
        alpha_ = (1.0f - fractionalPart) / (1.0f + fractionalPart);
        return static_cast<int32_t>(intergalPart);
    }
}

}