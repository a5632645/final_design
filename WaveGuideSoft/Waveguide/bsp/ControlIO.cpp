#include "ControlIO.hpp"

#include <array>

#include "stm32h7xx_hal.h"
#include "stm32h7xx_ll_exti.h"
#include "stm32h7xx_ll_gpio.h"

#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"

#include "SystemHook.hpp"
#include "MemAttributes.hpp"
#include "bsp/DebugIO.hpp"

// enacoder 1
#define ECA1_GPIO GPIOA
#define ECA1_PIN GPIO_PIN_3
#define ECA2_GPIO GPIOA
#define ECA2_PIN GPIO_PIN_2
#define ECA_IRQn EXTI3_IRQn
// enacoder 2
#define ECB1_GPIO GPIOA
#define ECB1_PIN GPIO_PIN_1
#define ECB2_GPIO GPIOA
#define ECB2_PIN GPIO_PIN_0
#define ECB_IRQn EXTI1_IRQn

namespace bsp {

static int32_t encoderValues_[CControlIO::kNumEncoders]{};
static uint32_t prevButtonState_{ 0xffffffff };
static uint32_t currButtonState_{ 0xffffffff };
static uint32_t tick_{};
static CControlIO::BtnCallback btnCallbacks_[CControlIO::kNumButtons]{};
static CControlIO::EncoderCallback encoderCallbacks_[CControlIO::kNumEncoders]{};

inline static void NullBtnCallback(ButtonEventArgs) {};
inline static void NullEncoderCallback(int32_t) {};

void CControlIO::Init() {
    /* 按键初始化 */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitTypeDef init{};
    init.Mode = GPIO_MODE_INPUT;
    init.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &init);

    /* 编码器初始化 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpioInit{};
    gpioInit.Alternate = 0;
    gpioInit.Mode = GPIO_MODE_INPUT;
    gpioInit.Pin = ECA2_PIN;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ECA2_GPIO, &gpioInit);
    gpioInit.Pin = ECB2_PIN;
    HAL_GPIO_Init(ECB2_GPIO, &gpioInit);
    gpioInit.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpioInit.Pin = ECA1_PIN;
    HAL_GPIO_Init(ECA1_GPIO, &gpioInit);
    gpioInit.Pin = ECB1_PIN;
    HAL_GPIO_Init(ECB1_GPIO, &gpioInit);
    HAL_NVIC_SetPriority(ECA_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(ECA_IRQn);
    HAL_NVIC_SetPriority(ECB_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(ECB_IRQn);

    // reset callbacks
    std::fill_n(btnCallbacks_, kNumButtons, NullBtnCallback);
    std::fill_n(encoderCallbacks_, kNumEncoders, NullEncoderCallback);
}

void CControlIO::WaitForNextEvent() {
    xTaskDelayUntil(&tick_, pdMS_TO_TICKS(kScanMs));
    prevButtonState_ = currButtonState_;
    currButtonState_ = LL_GPIO_ReadInputPort(GPIOE);
}

void CControlIO::ProcessButtonEvents() {
    uint32_t mask = 1 << 14;
    for (uint32_t i = 0; i < kNumButtons; ++i) {
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
            btnCallbacks_[i](state);
        }
        mask >>= 1;
    }
}

void CControlIO::ProcessEncoderEvents() {
    for (uint32_t i = 0; i < kNumEncoders; ++i) {
        if (encoderValues_[i] != 0) {
            encoderCallbacks_[i](encoderValues_[i]);
            encoderValues_[i] = 0;
        }
    }
}

void CControlIO::SetButtonCallback(ButtonId id, BtnCallback callback) {
    btnCallbacks_[static_cast<uint32_t>(id)] = callback;
}

void CControlIO::SetEncoderCallback(EncoderId id, EncoderCallback callback) {
    encoderCallbacks_[static_cast<uint32_t>(id)] = callback;
}

void CControlIO::UnsetButtonCallback(ButtonId id) {
    btnCallbacks_[static_cast<uint32_t>(id)] = NullBtnCallback;
}

void CControlIO::UnsetEncoderCallback(EncoderId id) {
    encoderCallbacks_[static_cast<uint32_t>(id)] = NullEncoderCallback;
}

void CControlIO::ResetButtonCallbacks() {
    std::fill_n(btnCallbacks_, kNumButtons, NullBtnCallback);
}

void CControlIO::ResetEncoderCallbacks() {
    std::fill_n(encoderCallbacks_, kNumEncoders, NullEncoderCallback);
}

static struct {
    bool flag1 : 1;
    bool init_nu1 : 1;
    bool flag2 : 1;
    bool init_nu2 : 1;
    bool flag3 : 1;
    bool init_nu3 : 1;
    bool flag4 : 1;
    bool init_nu4 : 1;
} encoderIrqFlags{};

void AddEncoderValue(EncoderId id, int32_t value) {
    auto idx = static_cast<uint32_t>(id);
    encoderValues_[idx] += value;
}

extern "C" void EXTI3_IRQHandler(void) {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_3);
    auto keya = LL_GPIO_ReadInputPort(ECA1_GPIO) & ECA1_PIN;
    auto keyb = LL_GPIO_ReadInputPort(ECA2_GPIO) & ECA2_PIN;
    if (!encoderIrqFlags.init_nu1 && !keya) {
        encoderIrqFlags.flag1 = false;
        if (keyb) {
            encoderIrqFlags.flag1 = true;
        }
        encoderIrqFlags.init_nu1 = true;
    }

    if (encoderIrqFlags.init_nu1 && keya) {
        if (!keyb && encoderIrqFlags.flag1) {
            AddEncoderValue(EncoderId::kParam, -1);
        }
        if (keyb && !encoderIrqFlags.flag1) {
            AddEncoderValue(EncoderId::kParam, 1);
        }
        encoderIrqFlags.init_nu1 = false;
    }
}

extern "C" void EXTI1_IRQHandler(void) {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);
    auto keya = LL_GPIO_ReadInputPort(ECB1_GPIO) & ECB1_PIN;
    auto keyb = LL_GPIO_ReadInputPort(ECB2_GPIO) & ECB2_PIN;

    if (!encoderIrqFlags.init_nu2 && !keya) {
        encoderIrqFlags.flag2 = false;
        if (keyb) {
            encoderIrqFlags.flag2 = true;
        }
        encoderIrqFlags.init_nu2 = true;
    }

    if (encoderIrqFlags.init_nu2 && keya) {
        if (!keyb && encoderIrqFlags.flag2) {
            AddEncoderValue(EncoderId::kValue, -1);
        }
        if (keyb && !encoderIrqFlags.flag2) {
            AddEncoderValue(EncoderId::kValue, 1);
        }
        encoderIrqFlags.init_nu2 = false;
    }
}

}
