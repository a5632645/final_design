#pragma once
#include <array>
#include <cstdint>

namespace dsp {

class DelayLine {
public:
    /* 在44.1k采样率可以装载A0以上的音符(不包括A0) */
    static constexpr int32_t kMaxLength = 2048;
    static constexpr int32_t kMask = kMaxLength - 1;

    void Init(float /*sampleRate*/) {}
    float Process(float in);
    void Push(float in);
    void ClearInternal();
    float GetLast();
    void SetDelay(int32_t delay);
    void SetDelayUncheck(int32_t delay);
    float GetDelay() const { return delay_; }
private:
    std::array<float, kMaxLength> buffer_{};
    int32_t writePos_ = 0;
    int32_t delay_ = 0;
};
    
} // namespace dsp
