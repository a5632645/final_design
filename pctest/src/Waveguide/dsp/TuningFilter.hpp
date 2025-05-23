#pragma once
#include <cstdint>

namespace dsp {

// one pole all pass filter
class TunningFilter {
public:
    void Init(float sampleRate) {}
    float Process(float in);
    /**
     * @brief 
     * @param delay 环路延迟
     * @return 还剩下多少延迟
     */
    int32_t SetDelay(float delay);
private:
    float latch_{};
    float alpha_{};
};

}