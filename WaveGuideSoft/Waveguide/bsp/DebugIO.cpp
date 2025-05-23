#include "DebugIO.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_ll_dma.h"

#include <algorithm>
#include <stdarg.h>
#include <stdio.h>

#include "MemAttributes.hpp"
#include "SystemHook.hpp"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

namespace bsp {

DebugIO Debug;

}

#define RED_Pin GPIO_PIN_0
#define RED_GPIO_Port GPIOC
#define GREEN_Pin GPIO_PIN_1
#define GREEN_GPIO_Port GPIOC
#define BLUE_Pin GPIO_PIN_2
#define BLUE_GPIO_Port GPIOC
extern uint32_t SystemCoreClock;

#define LED_GPIO GPIOB
#define LED_PIN GPIO_PIN_11

static void Led_Enable(bool on) {
    if (on) {
        HAL_GPIO_WritePin(LED_GPIO, LED_PIN, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin(LED_GPIO, LED_PIN, GPIO_PIN_RESET);
    }
}

namespace bsp {

static void LED_R(int state) {
    // HAL_GPIO_WritePin(RED_GPIO_Port, RED_Pin, static_cast<GPIO_PinState>(state));
}
static void LED_G(int state) {
    // HAL_GPIO_WritePin(GREEN_GPIO_Port, GREEN_Pin, static_cast<GPIO_PinState>(state));
}
static void LED_B(int state) {
    // HAL_GPIO_WritePin(BLUE_GPIO_Port, BLUE_Pin, static_cast<GPIO_PinState>(state));
}

static UART_HandleTypeDef huart_;
static DMA_HandleTypeDef hdma;

MEM_DMA_SRAMD1 static char sendBuffer[DebugIO::kMaxPrintfString + 2];
static SemaphoreHandle_t dmaSemHandle_ = NULL;
static StaticSemaphore_t dmaSem_;

// --------------------------------------------------------------------------------
// RTOS IO
// --------------------------------------------------------------------------------
void DebugIO::StartRTOSIO() {
    if (rtosInit_) {
        return;
    }

    /* 初始化DMA */
    {
        __HAL_RCC_DMA1_CLK_ENABLE();
        hdma.Instance = DMA1_Stream0;
        hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        hdma.Init.MemBurst = DMA_MBURST_SINGLE;
        hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma.Init.MemInc = DMA_MINC_ENABLE;
        hdma.Init.Mode = DMA_NORMAL;
        hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
        hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma.Init.Priority = DMA_PRIORITY_VERY_HIGH;
        // hdma.Init.Request = DMA_REQUEST_USART2_TX;
        hdma.Init.Request = DMA_REQUEST_USART1_TX;
        if (HAL_DMA_Init(&hdma) != HAL_OK) {
            DEVICE_ERROR("DebugIO", "HAL_DMA_Init failed");
        }
    }

    /* 链接DMA */
    {
        hdma.Parent = &huart_;
        huart_.hdmatx = &hdma;
    }

    /* 中断 */
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 10, 0);
    // HAL_NVIC_EnableIRQ(USART2_IRQn);
    // HAL_NVIC_SetPriority(USART2_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    HAL_NVIC_SetPriority(USART1_IRQn, 10, 0);

    /* 其他初始化 */
    dmaSemHandle_ = xSemaphoreCreateBinaryStatic(&dmaSem_);
    xSemaphoreGive(dmaSemHandle_);
}

usf::StringSpan DebugIO::GetRTOSPrintBuffer() {
    xSemaphoreTake(dmaSemHandle_, portMAX_DELAY);
    return usf::StringSpan(sendBuffer, kMaxPrintfString);
}

void DebugIO::WriteSerialPoll(usf::StringSpan span) {
    for (auto c : span) {
        while (__HAL_UART_GET_FLAG(&huart_, UART_FLAG_TXE) == RESET) {}
        huart_.Instance->TDR = c;
    }
}

void DebugIO::XWriteSerial(usf::StringSpan span, bool newLine) {
    uint32_t len = static_cast<uint32_t>(span.size());
    if (newLine) {
        char* end = span.end();
        len += 2;
        *end++ = '\n';
        *end++ = '\r';
    }

    if (auto res = HAL_UART_Transmit_DMA(&huart_, (uint8_t*)(sendBuffer), len);
        res != HAL_OK) {
        DEVICE_ERROR_CODE("DebugIO", "HAL_UART_Transmit_DMA failed", res);
    }
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef*) {
    xSemaphoreGiveFromISR(dmaSemHandle_, nullptr);
}

extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef*) {
    DEVICE_ERROR_CODE("DebugIO", "HAL_UART_ErrorCallback", HAL_UART_GetError(&huart_));
}

extern "C" void DMA1_Stream0_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma);
}

// extern "C" void USART2_IRQHandler(void) {
//     HAL_UART_IRQHandler(&huart_);
// }
extern "C" void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart_);
}

// --------------------------------------------------------------------------------
// API
// --------------------------------------------------------------------------------
void DebugIO::FlashErrorLight(uint32_t ms) {
    bool on = false;
    for (;;) {
        // HAL_GPIO_TogglePin(RED_GPIO_Port, RED_Pin);
        Led_Enable(on);
        on = !on;
        Wait(ms);
    }
}

void DebugIO::FlashErrorLightSequence(LedErrorSequence sequence) {
    for (;;) {
        for (uint32_t i = 0; i < sequence.numLong; ++i) {
            Led_Enable(true);
            Wait(sequence.longMs);
            Led_Enable(false);
            Wait(sequence.longMs);
        }
        for (uint32_t i = 0; i < sequence.numShort; ++i) {
            Led_Enable(true);
            Wait(sequence.shortMs);
            Led_Enable(false);
            Wait(sequence.shortMs);
        }
    }
}

void DebugIO::SetLed(bool r, bool g, bool b) {
    // LED_R(!r);
    // LED_G(!g);
    // LED_B(!b);
}

void DebugIO::ToggleLed(LedEnum led) {
    // switch (led) {
    //     case LedEnum::Red:
    //         HAL_GPIO_TogglePin(RED_GPIO_Port, RED_Pin);
    //         break;
    //     case LedEnum::Green:
    //         HAL_GPIO_TogglePin(GREEN_GPIO_Port, GREEN_Pin);
    //         break;
    //     case LedEnum::Blue:
    //         HAL_GPIO_TogglePin(BLUE_GPIO_Port, BLUE_Pin);
    //         break;
    // }
}

void DebugIO::SetLed(LedEnum led, bool on) {
    // switch (led) {
    //     case LedEnum::Red:
    //         LED_R(!on);
    //         break;
    //     case LedEnum::Green:
    //         LED_G(!on);
    //         break;
    //     case LedEnum::Blue:
    //         LED_B(!on);
    //         break;
    // }
}

void DebugIO::PrintStackAndRegisters() {
    if (!rtosInit_) {
        uint32_t* sp = reinterpret_cast<uint32_t*>(__get_MSP());;
        uint32_t** sp_end_addr = reinterpret_cast<uint32_t**>(SCB->VTOR);
        uint32_t* sp_end = *sp_end_addr;
    
        WriteLine("Stack:");
        for (; sp < sp_end; ++sp) {
            WriteLine("{:x}: {:x}", reinterpret_cast<uint32_t>(sp), *sp);
        }
        WriteLine("Registers:");
    }

    uint32_t val = 0;
    asm volatile ("mov %0, r0" : "=r" (val));
    WriteLine("R0: {:x}", val);
    asm volatile ("mov %0, r1" : "=r" (val));
    WriteLine("R1: {:x}", val);
    asm volatile ("mov %0, r2" : "=r" (val));
    WriteLine("R2: {:x}", val);
    asm volatile ("mov %0, r3" : "=r" (val));
    WriteLine("R3: {:x}", val);
    asm volatile ("mov %0, r4" : "=r" (val));
    WriteLine("R4: {:x}", val);
    asm volatile ("mov %0, r5" : "=r" (val));
    WriteLine("R5: {:x}", val);
    asm volatile ("mov %0, r6" : "=r" (val));
    WriteLine("R6: {:x}", val);
    asm volatile ("mov %0, r7" : "=r" (val));
    WriteLine("R7: {:x}", val);

    asm volatile ("mov %0, r8" : "=r" (val));
    WriteLine("R8: {:x}", val);
    asm volatile ("mov %0, r9" : "=r" (val));
    WriteLine("R9: {:x}", val);
    asm volatile ("mov %0, r10" : "=r" (val));
    WriteLine("R10: {:x}", val);
    asm volatile ("mov %0, r11" : "=r" (val));
    WriteLine("R11: {:x}", val);
    asm volatile ("mov %0, r12" : "=r" (val));
    WriteLine("R12: {:x}", val);

    asm volatile ("mov %0, pc" : "=r" (val));
    WriteLine("PC: {:x}", val);
    
    asm volatile ("mov %0, lr" : "=r" (val));
    WriteLine("LR: {:x}", val);

    WriteLine("End of registers");
}

void DebugIO::Init()
{
    {
        // __HAL_RCC_USART2_CLK_ENABLE();
        // huart_.Instance = USART2;
        __HAL_RCC_USART1_CLK_ENABLE();
        huart_.Instance = USART1;
        huart_.Init.BaudRate = 115200;
        huart_.Init.WordLength = UART_WORDLENGTH_8B;
        huart_.Init.StopBits = UART_STOPBITS_1;
        huart_.Init.Parity = UART_PARITY_NONE;
        huart_.Init.Mode = UART_MODE_TX;
        huart_.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart_.Init.OverSampling = UART_OVERSAMPLING_16;
        huart_.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
        huart_.Init.ClockPrescaler = UART_PRESCALER_DIV1;
        huart_.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
        HAL_UART_Init(&huart_);
    }
    {
        // // TX -> PA2
        // // RX -> PA3
        // __HAL_RCC_GPIOA_CLK_ENABLE();
        // GPIO_InitTypeDef init{
        //     .Pin = GPIO_PIN_2,
        //     .Mode = GPIO_MODE_AF_PP,
        //     .Pull = GPIO_NOPULL,
        //     .Speed = GPIO_SPEED_FREQ_HIGH,
        //     .Alternate = GPIO_AF7_USART2
        // };
        // HAL_GPIO_Init(GPIOA, &init);

        // TX -> PA9
        // RX -> PA10
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitTypeDef init{
            .Pin = GPIO_PIN_9,
            .Mode = GPIO_MODE_AF_PP,
            .Pull = GPIO_NOPULL,
            .Speed = GPIO_SPEED_FREQ_HIGH,
            .Alternate = GPIO_AF7_USART1
        };
        HAL_GPIO_Init(GPIOA, &init);

        // RX Disable
    }
    {
        // __HAL_RCC_GPIOC_CLK_ENABLE();
        // GPIO_InitTypeDef init{};
        // init.Pin = RED_Pin | GREEN_Pin | BLUE_Pin;
        // init.Mode = GPIO_MODE_OUTPUT_PP;
        // init.Pull = GPIO_NOPULL;
        // init.Speed = GPIO_SPEED_FREQ_HIGH;
        // HAL_GPIO_Init(GPIOC, &init);
    }
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitTypeDef init{};
        init.Pin = LED_PIN;
        init.Mode = GPIO_MODE_OUTPUT_PP;
        init.Pull = GPIO_NOPULL;
        init.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(LED_GPIO, &init);
    }
}

void DebugIO::InitLed() {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef init{};
    init.Pin = LED_PIN;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED_GPIO, &init);
}

void DebugIO::NewLine() {
    while (__HAL_UART_GET_FLAG(&huart_, UART_FLAG_TXE) == RESET) {}
    huart_.Instance->TDR = '\n';
    while (__HAL_UART_GET_FLAG(&huart_, UART_FLAG_TXE) == RESET) {}
    huart_.Instance->TDR = '\r';
}

uint32_t DebugIO::GetCpuSpeed() {
    return SystemCoreClock;
}

void DebugIO::Wait(uint32_t ms) {
    return Wait(ms, GetCpuSpeed());
}

void DebugIO::Wait(uint32_t ms, uint32_t cpuSpeed) {
    uint32_t ticks = ms * cpuSpeed / 1000;
    while(ticks) {
        --ticks;
        __NOP();
    }
}

void DebugIO::HalWait(uint32_t ms) {
    HAL_Delay(ms);
}

}