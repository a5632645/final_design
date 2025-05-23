#pragma once
#include <cstdint>
#include <algorithm>
#include <atomic>

namespace dsp {

struct ThreadSafeCallback {
    using MarkType = uint32_t;
    static constexpr MarkType kOne = static_cast<MarkType>(1);
    static constexpr MarkType kZero = static_cast<MarkType>(0);
    static constexpr uint32_t kNumBits = sizeof(MarkType) * 8;
    std::atomic<MarkType> down_{};
    std::atomic<MarkType> up_{};
    MarkType lastDown_{};
    MarkType lastUp_{};
    std::atomic<MarkType> down2_{};
    std::atomic<MarkType> up2_{};
    MarkType lastDown2_{};
    MarkType lastUp2_{};

    void lock();
    void unlock();

    using CallbackFunc = void(*)();
    static constexpr uint32_t kNumCallbacks = kNumBits * 4;
    CallbackFunc callbacks_[kNumCallbacks]{};

    uint32_t proxyCounter_ = 0;

    struct Proxy {
        ThreadSafeCallback& parent_;
        uint32_t index_;

        void SetCallback(CallbackFunc func) const {
            parent_.callbacks_[index_] = func;
        }

        void MarkDirty() const {
            parent_.MarkDirty(index_);
        }
    };
    [[nodiscard]] Proxy NewProxy();
    void HandleDirtyCallbacks();
    void MarkDirty(uint32_t index);
    void MarkAll();
    uint32_t GetProxyCounter();
};

} // namespace dsp
