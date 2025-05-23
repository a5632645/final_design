#include "Keyboard.hpp"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_exti.h"
#include "MemAttributes.hpp"
#include "bsp/DebugIO.hpp"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "utli/Clamp.hpp"
#include "utli/Map.hpp"

#define CS_GPIO GPIOC
#define CS_PIN GPIO_PIN_5
#define CS2_GPIO GPIOB
#define CS2_PIN GPIO_PIN_1
#define IRQ_GPIO GPIOB
#define IRQ_PIN GPIO_PIN_0
#define IRQ_IRQn EXTI0_IRQn
#define IRQ2_GPIO GPIOB
#define IRQ2_PIN GPIO_PIN_2
#define CONFIG_GPIO GPIOB
#define CONFIG_PIN GPIO_PIN_10
#define CONFIG2_GPIO GPIOE
#define CONFIG2_PIN GPIO_PIN_15

#define WRITE_READ_GPIO CONFIG_GPIO
#define WRITE_READ_PIN CONFIG_PIN
#define A0_GPIO IRQ2_GPIO
#define A0_PIN IRQ2_PIN
#define A1_GPIO CONFIG2_GPIO
#define A1_PIN CONFIG2_PIN
#define A2_GPIO CS2_GPIO
#define A2_PIN CS2_PIN
#define A3_GPIO CS_GPIO
#define A3_PIN CS_PIN

namespace bsp {

struct TransferData {
    uint8_t checkCode;
    uint8_t checkCode2;
    uint8_t adc;
    int8_t pos;

    bool IsValid() const {
        return checkCode == 0b11011000 && checkCode2 == 0b10101010 && adc > 30;
    }
};

static constexpr uint8_t kKeyboardMap[] {
    8, 10, 9, 11, 7, 6, 0, 2, 5, 4, 3, 1
};

static CKeyboard::KeyTransferData keyData_[CKeyboard::kNumKeys];
MEM_DMA_SRAMD3 static TransferData transferBuffer_;
static SPI_HandleTypeDef hspi_;
static DMA_HandleTypeDef hdma_;
static SemaphoreHandle_t spiSemHandle_ = nullptr;
static StaticSemaphore_t spiSem_;
static bool transferRequesing_ = false;
static bool writeRead_ = false;

void CKeyboard::Init(uint32_t dataRate) {
    dataRate_ = dataRate;
    
    // spi init
    __HAL_RCC_SPI6_CLK_ENABLE();
    hspi_.Instance = SPI6;
    HAL_SPI_DeInit(&hspi_);

    hspi_.Init.Mode = SPI_MODE_MASTER;
    hspi_.Init.Direction = SPI_DIRECTION_1LINE;
    hspi_.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi_.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi_.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi_.Init.NSS = SPI_NSS_SOFT;
    hspi_.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    hspi_.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi_.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi_.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi_.Init.CRCPolynomial = 10;
    hspi_.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    hspi_.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_03CYCLE;
    hspi_.Init.IOSwap = SPI_IO_SWAP_ENABLE;
    HAL_SPI_Init(&hspi_);

    HAL_NVIC_EnableIRQ(SPI6_IRQn);
    HAL_NVIC_SetPriority(SPI6_IRQn, 10, 0);

    // spi gpio init
    // PB4 -> MISO
    // PB3 -> SCK
    // PD3 -> CS
    // PD1 -> IRQ
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef gpioInit{};
    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Pin = GPIO_PIN_3;
    gpioInit.Alternate = GPIO_AF8_SPI6;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpioInit);

    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Pin = GPIO_PIN_4;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    gpioInit.Alternate = GPIO_AF8_SPI6;
    HAL_GPIO_Init(GPIOB, &gpioInit);

    // pins config
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // address pins
    gpioInit.Alternate = 0;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    gpioInit.Pin = A0_PIN;
    HAL_GPIO_Init(A0_GPIO, &gpioInit);
    gpioInit.Pin = A1_PIN;
    HAL_GPIO_Init(A1_GPIO, &gpioInit);
    gpioInit.Pin = A2_PIN;
    HAL_GPIO_Init(A2_GPIO, &gpioInit);
    gpioInit.Pin = A3_PIN;
    HAL_GPIO_Init(A3_GPIO, &gpioInit);
    // set to channel 12
    HAL_GPIO_WritePin(A0_GPIO, A0_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(A1_GPIO, A1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(A2_GPIO, A2_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(A3_GPIO, A3_PIN, GPIO_PIN_SET);
    // read write pin
    gpioInit.Pin = WRITE_READ_PIN;
    HAL_GPIO_Init(WRITE_READ_GPIO, &gpioInit);
    
    // irq
    gpioInit.Alternate = 0;
    gpioInit.Pin = IRQ_PIN;
    gpioInit.Pull = GPIO_PULLDOWN;
    gpioInit.Mode = GPIO_MODE_IT_FALLING;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(IRQ_GPIO, &gpioInit);
    HAL_NVIC_EnableIRQ(IRQ_IRQn);
    HAL_NVIC_SetPriority(IRQ_IRQn, 10, 0);

    // spi dma init
    __HAL_RCC_BDMA_CLK_ENABLE();
    hdma_.Instance = BDMA_Channel0;
    hdma_.Init.Request = BDMA_REQUEST_SPI6_RX;
    hdma_.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_.Init.MemInc = DMA_MINC_ENABLE;
    hdma_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_.Init.Mode = DMA_NORMAL;
    hdma_.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_);

    hspi_.hdmarx = &hdma_;
    hdma_.Parent = &hspi_;

    HAL_NVIC_EnableIRQ(BDMA_Channel0_IRQn);
    HAL_NVIC_SetPriority(BDMA_Channel0_IRQn, 10, 0);
    
    // semaphore init
    spiSemHandle_ = xSemaphoreCreateBinaryStatic(&spiSem_);
    vTaskDelay(1000); // 等待CH552G开机
}

void CKeyboard::UpdateData() {
    // reciving data
    HAL_GPIO_WritePin(WRITE_READ_GPIO, WRITE_READ_PIN, GPIO_PIN_RESET); // read
    HAL_GPIO_WritePin(A3_GPIO, A3_PIN, GPIO_PIN_RESET);
    Keyboard_Read(4);
    HAL_GPIO_WritePin(A2_GPIO, A2_PIN, GPIO_PIN_RESET);
    Keyboard_Read(0);
    HAL_GPIO_WritePin(A0_GPIO, A0_PIN, GPIO_PIN_SET);
    Keyboard_Read(1);
    HAL_GPIO_WritePin(A1_GPIO, A1_PIN, GPIO_PIN_SET);
    Keyboard_Read(3);
    HAL_GPIO_WritePin(A0_GPIO, A0_PIN, GPIO_PIN_RESET);
    Keyboard_Read(2);
    HAL_GPIO_WritePin(A2_GPIO, A2_PIN, GPIO_PIN_SET);
    Keyboard_Read(6);
    HAL_GPIO_WritePin(A0_GPIO, A0_PIN, GPIO_PIN_SET);
    Keyboard_Read(7);
    HAL_GPIO_WritePin(A1_GPIO, A1_PIN, GPIO_PIN_RESET);
    Keyboard_Read(5);
    HAL_GPIO_WritePin(A3_GPIO, A3_PIN, GPIO_PIN_SET); // 13
    HAL_GPIO_WritePin(A2_GPIO, A2_PIN, GPIO_PIN_RESET);
    Keyboard_Read(9);
    HAL_GPIO_WritePin(A0_GPIO, A0_PIN, GPIO_PIN_RESET);
    Keyboard_Read(8);
    HAL_GPIO_WritePin(A1_GPIO, A1_PIN, GPIO_PIN_SET);
    Keyboard_Read(10);
    HAL_GPIO_WritePin(A0_GPIO, A0_PIN, GPIO_PIN_SET);
    Keyboard_Read(11);
    HAL_GPIO_WritePin(A2_GPIO, A2_PIN, GPIO_PIN_SET); // 15
    HAL_GPIO_WritePin(A1_GPIO, A1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(A0_GPIO, A0_PIN, GPIO_PIN_RESET); // 12

    // filter data
    for (uint32_t i = 0; i < kNumKeys; ++i) {
        auto f = filteredPositions_[i] * dataFilterAlphas_ + keyData_[i].fingerPostion * (1.0f - dataFilterAlphas_);
        f = std::clamp(f, -127.0f, 127.0f);
        filteredPositions_[i] = static_cast<int8_t>(f);
    }
}

void CKeyboard::ProcessData() {
    uint16_t mask = 1;
    lastKeyPressFlags_ = keyPressFlags_;
    for (uint32_t i = 0; i < kNumKeys; ++i) {
        auto& data = keyData_[i];
        if (data.adcValue < 10) {
            mask <<= 1;
            continue;
        }

        if (data.adcValue > adcUpValue_ && (mask & keyPressFlags_)) {
            // key release
            keyPressFlags_ &= ~mask;
            keyUpCallback_(i);
        }
        else if (data.adcValue < adcDownValue_ && !(mask & keyPressFlags_)) {
            // key press
            keyPressFlags_ |= mask;
            keyDownCallback_(i);
        }
        mask <<= 1;
    }
}

std::span<CKeyboard::KeyTransferData> CKeyboard::GetDatas() {
    return std::span<KeyTransferData>(keyData_, kNumKeys);
}

uint8_t CKeyboard::GetPressure(uint8_t keyIndex) const {
    return utli::Map(127, 0, adcMinValue_, adcDownValue_, keyData_[keyIndex].adcValue);
}

int8_t CKeyboard::GetFingerPosition(uint8_t keyIndex) const {
    return filteredPositions_[keyIndex];
}

uint8_t CKeyboard::GetAdcValue(uint8_t keyIndex) const {
    return keyData_[keyIndex].adcValue;
}

void CKeyboard::SetDataFilterAlphas(float alpha) {
    dataFilterAlphas_ = std::clamp(alpha, 0.0f, 1.0f);
}

bool CKeyboard::IsKeyPressed(uint8_t idx) const {
    // return keyPressFlags_ & (1 << kNote2KeyIdxTable[idx]);
    return keyPressFlags_ & (1 << idx);
}

void CKeyboard::Keyboard_Read(uint8_t idx) {
    transferRequesing_ = true;
    writeRead_ = false;
    if (xSemaphoreTake(spiSemHandle_, pdMS_TO_TICKS(3)) == pdTRUE) { // not timeout
        if (transferBuffer_.IsValid()) {
            idx = kKeyboardMap[idx];
            keyData_[idx].adcValue = transferBuffer_.adc;
            keyData_[idx].fingerPostion = transferBuffer_.pos;
        }
    }
}

void CKeyboard::Keyboard_Write(uint8_t idx) {
    transferRequesing_ = true;
    writeRead_ = true;
    if (xSemaphoreTake(spiSemHandle_, pdMS_TO_TICKS(3)) == pdTRUE) { // not timeout
        // do nothing
    }
}

extern "C" void EXTI0_IRQHandler(void) {
    if (transferRequesing_) {
        transferRequesing_ = false;
        if (writeRead_) { // write
            hspi_.TxCpltCallback = [](SPI_HandleTypeDef*) {
                xSemaphoreGiveFromISR(spiSemHandle_, nullptr);
            };
            HAL_SPI_Transmit_IT(&hspi_, reinterpret_cast<uint8_t *>(&transferBuffer_), sizeof(transferBuffer_));
        }
        else { // read
            hspi_.RxCpltCallback = [](SPI_HandleTypeDef*) {
                xSemaphoreGiveFromISR(spiSemHandle_, nullptr);
            };
            HAL_SPI_Receive_IT(&hspi_, reinterpret_cast<uint8_t *>(&transferBuffer_), sizeof(transferBuffer_));
        }
    }
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
}

extern "C" void BDMA_Channel0_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_);
}

extern "C" void SPI6_IRQHandler(void) {
    HAL_SPI_IRQHandler(&hspi_);
}

}
