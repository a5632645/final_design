#include "ControlIO.hpp"
#include <array>
#include <thread>
#include <chrono>
#include <raylib.h>

namespace bsp {

static int32_t encoderValues_[CControlIO::kNumEncoders]{};
static uint32_t prevButtonState_{};
static uint32_t currButtonState_{};
static uint32_t tick_{};
static CControlIO::BtnCallback btnCallbacks_[CControlIO::kNumButtons]{};
static CControlIO::EncoderCallback encoderCallbacks_[CControlIO::kNumEncoders]{};

static constexpr std::array kKeys {
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_HOME,
    KEY_N,
    KEY_M
};
static_assert(kKeys.size() == CControlIO::kNumButtons);

void CControlIO::Init() {
    for (auto& c : btnCallbacks_) {
        c = nullptr;
    }
    for (auto& c : encoderCallbacks_) {
        c = nullptr;
    }
}

double upKeyPressTime = 0;
double downKeyPressTime = 0;
double leftKeyPressTime = 0;
double rightKeyPressTime = 0;
void CControlIO::WaitForNextEvent() {
    if (IsKeyPressed(KEY_UP)) {
        encoderValues_[1]++;
        upKeyPressTime = GetTime();
    }
    else if (IsKeyPressed(KEY_DOWN)) {
        encoderValues_[1]--;
        downKeyPressTime = GetTime();
    }
    if (IsKeyPressed(KEY_LEFT)) {
        encoderValues_[0]--;
        leftKeyPressTime = GetTime();
    }
    else if (IsKeyPressed(KEY_RIGHT)) {
        encoderValues_[0]++;
        rightKeyPressTime = GetTime();
    }

    if (GetTime() - upKeyPressTime > 1 && IsKeyDown(KEY_UP)) {
        encoderValues_[1]++;
    }
    if (GetTime() - downKeyPressTime > 1 && IsKeyDown(KEY_DOWN)) {
        encoderValues_[1]--;
    }
    if (GetTime() - leftKeyPressTime > 1 && IsKeyDown(KEY_LEFT)) {
        encoderValues_[0]--;
    }
    if (GetTime() - rightKeyPressTime > 1 && IsKeyDown(KEY_RIGHT)) {
        encoderValues_[0]++;
    }

    static auto lastTimePoint = GetTime();
    if (GetTime() - lastTimePoint < kScanMs / 1000.0) {
        return;
    }
    lastTimePoint = GetTime();

    prevButtonState_ = currButtonState_;
    currButtonState_ = 0;
    for (uint32_t i = 0; i < kNumButtons; ++i) {
        if (IsKeyDown(kKeys[i])) {
            currButtonState_ |= 1 << i;
        }
    }
}

void CControlIO::ProcessButtonEvents() {
    for (uint32_t i = 0; i < kNumButtons; ++i) {
        auto mask = 1 << i;
        auto changed = currButtonState_ ^ prevButtonState_;
        auto pressed = currButtonState_ & mask;
        auto state = ButtonState::kAttack;
        if (changed & mask) {
            if (pressed) {
                state = ButtonState::kAttack;
            }
            else {
                state = ButtonState::kRelease;
            }
        }
        else {
            if (pressed) {
                state = ButtonState::kHold;
            }
            else {
                state = ButtonState::kInit;
            }
        }
        if (state != ButtonState::kInit) {
            if (btnCallbacks_[i] != nullptr) {
                btnCallbacks_[i](state);
            }
        }
    }
}

void CControlIO::ProcessEncoderEvents() {
    for (uint32_t i = 0; i < kNumEncoders; ++i) {
        if (encoderValues_[i] != 0) {
            if (encoderCallbacks_[i] != nullptr) {
                encoderCallbacks_[i](encoderValues_[i]);
            }
            encoderValues_[i] = 0;
        }
    }
}

ButtonState CControlIO::GetButtonState(ButtonId id) {
    auto idx = static_cast<uint32_t>(id);
    auto mask = 1 << idx;
    auto changed = currButtonState_ ^ prevButtonState_;
    auto pressed = currButtonState_ & mask;
    if (changed & mask) {
        if (pressed) {
            return ButtonState::kAttack;
        }
        else {
            return ButtonState::kRelease;
        }
    }
    else {
        if (pressed) {
            return ButtonState::kHold;
        }
        else {
            return ButtonState::kInit;
        }
    }
}

int32_t CControlIO::GetEncoderValue(EncoderId id) {
    auto idx = static_cast<uint32_t>(id);
    return encoderValues_[idx];
}

void CControlIO::SetButtonCallback(ButtonId id, BtnCallback callback) {
    btnCallbacks_[static_cast<uint32_t>(id)] = callback;
}

void CControlIO::SetEncoderCallback(EncoderId id, EncoderCallback callback) {
    encoderCallbacks_[static_cast<uint32_t>(id)] = callback;
}

void CControlIO::UnsetButtonCallback(ButtonId id) {
    btnCallbacks_[static_cast<uint32_t>(id)] = nullptr;
}

void CControlIO::UnsetEncoderCallback(EncoderId id) {
    encoderCallbacks_[static_cast<uint32_t>(id)] = nullptr;
}

void CControlIO::ResetButtonCallbacks() {
    for (auto& c : btnCallbacks_) {
        c = nullptr;
    }
}

void CControlIO::ResetEncoderCallbacks() {
    for (auto& c : encoderCallbacks_) {
        c = nullptr;
    }
}
}

