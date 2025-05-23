#pragma once
#include <cstdint>

namespace bsp {

enum class EncoderId : uint8_t {
    kParam = 0,
    kValue,
    kNumEncoders
};

enum class ButtonId : uint8_t {
    kOctSub = 0,
    kOctAdd,
    kBtn0,
    kBtn1,
    kBtn2,
    kBtn3,
    kBtn4,
    kBtnMain,
    kBtnValue,
    kBtnParam,
    kNumButtons
};

enum ButtonState : uint8_t {
    kInit = 0,
    kAttack,
    kHold,
    kRelease,
};

struct ButtonEventArgs {
    ButtonState state;

    ButtonEventArgs(ButtonState state) : state(state) {}
    bool IsAttack() const { return state == ButtonState::kAttack; }
    bool IsHold() const { return state == ButtonState::kHold; }
    bool IsRelease() const { return state == ButtonState::kRelease; }
};

class CControlIO {
public:
    static constexpr uint32_t kNumButtons = static_cast<uint32_t>(ButtonId::kNumButtons);
    static constexpr uint32_t kNumEncoders = static_cast<uint32_t>(EncoderId::kNumEncoders);
    static constexpr uint32_t kScanMs = 50;

    using BtnCallback = void(*)(ButtonEventArgs state);
    using EncoderCallback = void(*)(int32_t dvalue);

    void Init();
    void WaitForNextEvent();
    void ProcessButtonEvents();
    void ProcessEncoderEvents();

    void SetButtonCallback(ButtonId id, BtnCallback callback);
    void SetEncoderCallback(EncoderId id, EncoderCallback callback);
    void UnsetButtonCallback(ButtonId id);
    void UnsetEncoderCallback(EncoderId id);
    void ResetButtonCallbacks();
    void ResetEncoderCallbacks();
};

namespace internal {
struct InternalControlIO {
    inline static CControlIO instance;
};
}

static CControlIO& ControlIO = internal::InternalControlIO::instance;

}
