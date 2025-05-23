#include "SafeCallback.hpp"
#include "FreeRTOS.h"

namespace dsp {

void ThreadSafeCallback::lock() {
    portENTER_CRITICAL();
}

void ThreadSafeCallback::unlock() {
    portEXIT_CRITICAL();
}

ThreadSafeCallback::Proxy ThreadSafeCallback::NewProxy() {
    return ThreadSafeCallback::Proxy{ *this, proxyCounter_++ };
}

void ThreadSafeCallback::HandleDirtyCallbacks() {
    lock();
    lastDown_ = down_;
    lastUp_ = up_;
    down_ = 0;
    up_ = 0;
    lastDown2_ = down2_;
    lastUp2_ = up2_;
    down2_ = 0;
    up2_ = 0;
    unlock();
    while (lastDown_) {
        auto index = __builtin_ctzl(lastDown_);
        if (callbacks_[index] != nullptr) {
            callbacks_[index]();
        }
        lastDown_&= ~(kOne << index);
    }
    while (lastUp_) {
        auto index = __builtin_ctzl(lastUp_);
        auto no = index + kNumBits;
        if (callbacks_[no] != nullptr) {
            callbacks_[no]();
        }
        lastUp_ &= ~(kOne << index);
    }
    while (lastDown2_) {
        auto index = __builtin_ctzl(lastDown2_);
        auto no = index + kNumBits * 2;
        if (callbacks_[no] != nullptr) {
            callbacks_[no]();
        }
        lastDown2_&= ~(kOne << index);
    }
    while (lastUp2_) {
        auto index = __builtin_ctzl(lastUp2_);
        auto no = index + kNumBits * 3;
        if (callbacks_[no] != nullptr) {
            callbacks_[no]();
        }
        lastUp2_ &= ~(kOne << index);
    }
}

void ThreadSafeCallback::MarkDirty(uint32_t index) {
    lock();
    if (index < kNumBits) down_ |= kOne << index;
    else if (index < kNumBits * 2) up_ |= kOne << (index - kNumBits);
    else if (index < kNumBits * 3) down2_ |= kOne << (index - kNumBits * 2);
    else up2_ |= kOne << (index - kNumBits * 3);
    unlock();
}

void ThreadSafeCallback::MarkAll() {
    lock();
    down2_ = ~kZero;
    up2_ = ~kZero;
    up_ = ~kZero;
    down_ = ~kZero;
    unlock();
}

uint32_t ThreadSafeCallback::GetProxyCounter() {
    return proxyCounter_;
}

}