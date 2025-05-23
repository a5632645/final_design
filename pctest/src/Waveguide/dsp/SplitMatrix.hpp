#pragma once

namespace dsp {

class SplitMatrix {
public:
    void Push(float in0, float in1);
    float GetOut0() const;
    float GetOut1() const;
private:
    float latch0_{};
    float latch1_{};
    float out0_{};
    float out1_{};
};

}