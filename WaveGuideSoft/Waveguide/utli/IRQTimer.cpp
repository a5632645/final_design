#include "IRQTimer.hpp"

namespace utli {

void CIRQTimer::Tick() {
    auto* p = this;
    while (p != nullptr) {
        if (!p->disabled_) {
            p->tick_++;
            if (p->tick_ >= p->period_) {
                p->tick_ = 0;
                p->callback_(*p);
            }
        }
        p = p->next_;
    }
}

void CIRQTimer::Remove(CIRQTimer& timer) {
    if (timer.prev_ != nullptr) {
        timer.prev_->next_ = timer.next_;
    }
    if (timer.next_ != nullptr) {
        timer.next_->prev_ = timer.prev_;
    }
    timer.next_ = nullptr;
    timer.prev_ = nullptr;
}

void CIRQTimer::Add(CIRQTimer& timer) {
    if (next_ != nullptr) {
        next_->prev_ = &timer;
    }
    timer.next_ = next_;
    timer.prev_ = this;
    next_ = &timer;
}

}