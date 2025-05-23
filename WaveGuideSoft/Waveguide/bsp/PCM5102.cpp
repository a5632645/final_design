#include "PCM5102.hpp"

#include "stm32h7xx_hal.h"

#include "MemAttributes.hpp"
#include "SystemHook.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

namespace bsp {

static I2S_HandleTypeDef hi2s_;
static DMA_HandleTypeDef hdma_;

MEM_DMA_SRAMD1 static StereoSample dmaBuffer_[CPCM5102::kBufferSize];
static volatile uint32_t offset_ = 0;
static SemaphoreHandle_t dmaSemHandle_ = NULL;
static StaticSemaphore_t dmaSem_;
constexpr auto kGenSize = CPCM5102::kBlockSize;

// --------------------------------------------------------------------------------
// public
// --------------------------------------------------------------------------------
void CPCM5102::Init() {
    // i2s init
    __HAL_RCC_SPI1_CLK_ENABLE();
    hi2s_.Instance = SPI1;
    hi2s_.Init.Mode = I2S_MODE_MASTER_TX;
    hi2s_.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s_.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s_.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    hi2s_.Init.AudioFreq = kSampleRate;
    hi2s_.Init.CPOL = I2S_CPOL_LOW;
    hi2s_.Init.FirstBit = I2S_FIRSTBIT_MSB;
    hi2s_.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
    hi2s_.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_LEFT;
    hi2s_.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_DISABLE;
    HAL_I2S_Init(&hi2s_);

    // dma init
    __HAL_RCC_DMA1_CLK_ENABLE();
    hdma_.Instance = DMA1_Stream5;
    hdma_.Init.Request = DMA_REQUEST_SPI1_TX;
    hdma_.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_.Init.MemInc = DMA_MINC_ENABLE;
    hdma_.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_.Init.Mode = DMA_CIRCULAR;
    hdma_.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_);

    hi2s_.hdmatx = &hdma_;
    hdma_.Parent = &hi2s_;

    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 6, 0);

    // i2s gpio init
    // PC4 -> MCLK
    // PA5 -> BCK
    // PA4-> WS
    // PA7 -> SDO
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef gpioCInit{};
    gpioCInit.Alternate = GPIO_AF5_SPI1;
    gpioCInit.Mode = GPIO_MODE_AF_PP;
    gpioCInit.Pin = GPIO_PIN_4;
    gpioCInit.Pull = GPIO_NOPULL;
    gpioCInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &gpioCInit);

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpioAInit{};
    gpioAInit.Alternate = GPIO_AF5_SPI1;
    gpioAInit.Mode = GPIO_MODE_AF_PP;
    gpioAInit.Pin = GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_7;
    gpioAInit.Pull = GPIO_NOPULL;
    gpioAInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpioAInit);

    // os param init
    dmaSemHandle_ = xSemaphoreCreateBinaryStatic(&dmaSem_);
    std::fill_n(dmaBuffer_, kBufferSize, StereoSample{});
}

void CPCM5102::Start() {
    HAL_I2S_Transmit_DMA(&hi2s_, (uint16_t*)dmaBuffer_, kBufferSize * 2);
}

void CPCM5102::Stop() {
    HAL_I2S_DMAStop(&hi2s_);
}

void CPCM5102::DeInit() {
    HAL_I2S_DeInit(&hi2s_);
    HAL_DMA_DeInit(&hdma_);
    dmaSemHandle_ = NULL;
}

std::span<StereoSample> CPCM5102::GetNextBlock() {
    xSemaphoreTake(dmaSemHandle_, portMAX_DELAY);
    return std::span<StereoSample>(dmaBuffer_ + offset_, kGenSize);
}

// --------------------------------------------------------------------------------
// ISR
// --------------------------------------------------------------------------------
extern "C" void DMA1_Stream5_IRQHandler() {
    HAL_DMA_IRQHandler(&hdma_);
}

extern "C" void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef*) {
    offset_ = 0;
    xSemaphoreGiveFromISR(dmaSemHandle_, nullptr);
}

extern "C" void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef*) {
    offset_ = kGenSize;
    xSemaphoreGiveFromISR(dmaSemHandle_, nullptr);
}

extern "C" void HAL_I2S_ErrorCallback(I2S_HandleTypeDef* hi2s) {
    DEVICE_ERROR_CODE("AudioOut", "HAL_I2S_ErrorCallback", hi2s->ErrorCode);
}

}
