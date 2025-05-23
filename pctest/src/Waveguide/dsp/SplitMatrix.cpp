#include "SplitMatrix.hpp"

namespace dsp {

void SplitMatrix::Push(float in0, float in1) {
    // out0_ = (in0 + latch0_) / 2 + (in1 - latch1_) / 2;
    // out1_ = (in0 - latch0_) / 2 + (in1 + latch1_) / 2;
    out0_ = (in0 - latch0_) / 2 + (in1 + latch1_) / 2;
    out1_ = (in0 + latch0_) / 2 + (in1 - latch1_) / 2;
    latch0_ = in0;
    latch1_ = in1;
}


float SplitMatrix::GetOut0() const {
    return out0_;
}

float SplitMatrix::GetOut1() const {
    return out1_;
}

}