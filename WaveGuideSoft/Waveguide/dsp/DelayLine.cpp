#include "DelayLine.hpp"

namespace dsp {

float DelayLine::Process(float in) {
    writePos_++;
    writePos_ &= kMask;
    buffer_[writePos_] = in;
    return GetLast();
}

void DelayLine::Push(float in) {
    writePos_++;
    writePos_ &= kMask;
    buffer_[writePos_] = in;
}

void DelayLine::ClearInternal() {
    std::fill_n(buffer_.begin(), kMaxLength, 0);
}

float DelayLine::GetLast() {
    auto readIdx = writePos_ - delay_;
    readIdx &= kMask;
    return buffer_[readIdx];
}

void DelayLine::SetDelay(int32_t delay) {
    if (delay < 0) delay = 0;
    if (delay > kMaxLength) delay = kMaxLength;
    delay_ = static_cast<int32_t>(delay);
}

void DelayLine::SetDelayUncheck(int32_t delay) {
    delay_ = delay & kMask;
}

}