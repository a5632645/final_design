#pragma once

namespace dsp {

class DCBlocker {
public:
    void Init(float /*sampleRate*/) {}
    float Process(float in) {
        auto t = in - latch_;
        latch_ = in;
        auto y = t + 0.999f * latch1_;
        latch1_ = y;
        return y;
    }
    void ClearInternal() {
        latch_ = 0;
        latch1_ = 0;
    }
private:
    float latch_ = 0;
    float latch1_ = 0;
};

class DCBlocker995 {
public:
    void Init(float /*sampleRate*/) {}
    float Process(float in) {
        auto t = in - latch_;
        latch_ = in;
        auto y = t + 0.99f * latch1_;
        latch1_ = y;
        return y;
    }
    void ClearInternal() {
        latch_ = 0;
        latch1_ = 0;
    }
private:
    float latch_ = 0;
    float latch1_ = 0;
};

}