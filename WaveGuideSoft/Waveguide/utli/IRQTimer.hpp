#pragma once
#include <cstdint>

namespace utli {

class CIRQTimer {
public:
    static constexpr uint32_t kPeriodMs = 1;
    static constexpr uint32_t TickToMs(uint32_t tick) { return tick * kPeriodMs; }
    static constexpr uint32_t MsToTick(uint32_t ms) { return ms / kPeriodMs; }

    void Tick();
    void Add(CIRQTimer& timer);
    void Remove(CIRQTimer& timer);
    void SetCallback(void (*tick)(CIRQTimer&)) { callback_ = tick; }
    void SetPeriod(uint32_t ms) { period_ = MsToTick(ms); }
    void Enable() { disabled_ = false; }
    void Disable() { disabled_ = true; }
private:
    CIRQTimer* next_{};
    CIRQTimer* prev_{};
    bool disabled_{true};
    uint32_t period_{};
    uint32_t tick_{};
    void (*callback_)(CIRQTimer&) {};
};

namespace internal {
struct GlobalIRQTimer {
    inline static CIRQTimer timer;
};
}

static auto& IRQTimer = internal::GlobalIRQTimer::timer;

}