#include "UC1638.hpp"
#include "stm32h7xx_hal.h"
#include "DebugIO.hpp"

#include "MemAttributes.hpp"
#include "SystemHook.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LCD_RESET_GPIO GPIOD
#define LCD_RESET_PIN GPIO_PIN_1
#define LCD_DC_GPIO GPIOD
#define LCD_DC_PIN GPIO_PIN_0
#define LCD_CS_GPIO GPIOA
#define LCD_CS_PIN GPIO_PIN_15
#define LCD_BK_GPIO GPIOC
#define LCD_BK_PIN GPIO_PIN_11
#define LCD_SDA_GPIO GPIOC
#define LCD_SDA_PIN GPIO_PIN_12
#define LCD_SCL_GPIO GPIOC
#define LCD_SCL_PIN GPIO_PIN_10

#if 0

namespace bsp {

static SPI_HandleTypeDef hspi2_;
static DMA_HandleTypeDef hdma_;
MEM_DMA_SRAMD1 static uint8_t displayBuffer[OLEDDisplay::kBufferSize];
static StaticSemaphore_t dmaSem_;
static SemaphoreHandle_t dmaSemHandle_{ nullptr };

void CUC1638::Init() {
    // spi init
    // __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_SPI3_CLK_ENABLE();
    // hspi2_.Instance = SPI2;
    hspi2_.Instance = SPI3;
    HAL_SPI_DeInit(&hspi2_);

    hspi2_.Init.Mode = SPI_MODE_MASTER;
    hspi2_.Init.Direction = SPI_DIRECTION_2LINES_TXONLY;
    hspi2_.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2_.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2_.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2_.Init.NSS = SPI_NSS_SOFT;
    hspi2_.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi2_.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2_.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2_.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2_.Init.CRCPolynomial = 10;
    HAL_SPI_Init(&hspi2_);

    // msp init
    // __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitTypeDef gpioInit;
    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Pull = GPIO_NOPULL;
    // gpioInit.Pin = GPIO_PIN_10 | GPIO_PIN_15;
    gpioInit.Pin = GPIO_PIN_10 | GPIO_PIN_12;
    gpioInit.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    // gpioInit.Alternate = GPIO_AF5_SPI2;
    gpioInit.Alternate = GPIO_AF6_SPI3;
    // HAL_GPIO_Init(GPIOB, &gpioInit);
    HAL_GPIO_Init(GPIOC, &gpioInit);

    // B14 -> CS new:A15
    // B12 -> DC new:D0
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    // gpioInit.Pin = GPIO_PIN_12 | GPIO_PIN_14;
    gpioInit.Alternate = 0;
    // HAL_GPIO_Init(GPIOB, &gpioInit);
    gpioInit.Pin = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOA, &gpioInit);

    gpioInit.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOD, &gpioInit);

    // A1 -> BK new:C11
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    // gpioInit.Pin = GPIO_PIN_1;
    // HAL_GPIO_Init(GPIOA, &gpioInit);
    gpioInit.Pin = GPIO_PIN_11;
    HAL_GPIO_Init(GPIOC, &gpioInit);

    // spi it init
    // HAL_NVIC_EnableIRQ(SPI2_IRQn);
    // HAL_NVIC_SetPriority(SPI2_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(SPI3_IRQn);
    HAL_NVIC_SetPriority(SPI3_IRQn, 10, 0);

    // D8 -> reset new:D1
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    // gpioInit.Pin = GPIO_PIN_8;
    gpioInit.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOD, &gpioInit);

    // dma init
    __HAL_RCC_DMA1_CLK_ENABLE();
    hdma_.Instance = DMA1_Stream1;
    // hdma_.Init.Request = DMA_REQUEST_SPI2_TX;
    hdma_.Init.Request = DMA_REQUEST_SPI3_TX;
    hdma_.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_.Init.MemInc = DMA_MINC_ENABLE;
    hdma_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_.Init.Mode = DMA_NORMAL;
    hdma_.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_);

    hspi2_.hdmatx = &hdma_;
    hdma_.Parent = &hspi2_;

    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 10, 0);

    // display init
    display.SetDisplayBuffer(displayBuffer);
    dmaSemHandle_ = xSemaphoreCreateBinaryStatic(&dmaSem_);
}

void CUC1638::UpdateScreen() {
    _WriteCommand(0x04);//set column Address
    _WriteData(0x00);
    _WriteCommand(0x60);//set page Address
    _WriteCommand(0x70);

    uint8_t cmd = 0x01;
    _DC(false);
    _CS(true);
    HAL_SPI_Transmit(&hspi2_, &cmd, 1, 1000);
    _DC(true);
    hspi2_.TxCpltCallback = [](SPI_HandleTypeDef* hspi) { xSemaphoreGiveFromISR(dmaSemHandle_, nullptr); };
    HAL_SPI_Transmit_DMA(&hspi2_, display.getDisplayBuffer(), display.kBufferSize);
    xSemaphoreTake(dmaSemHandle_, portMAX_DELAY);
    _CS(false);
}

void CUC1638::SetBKlight(bool on) {
    if (on) {
        // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET);
    } else {
        // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);
    }
}

void CUC1638::SetMirror(bool x, bool y) {
    uint8_t xv = x ? 0b10 : 0;
    uint8_t yv = y ? 0b100 : 0;
    _WriteCommand(0xc0 | xv | yv);
}

void CUC1638::HardReset() {
    // ground is reset
    // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_RESET);
    vTaskDelay(std::max<TickType_t>(1, pdMS_TO_TICKS(10)));
    // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
    vTaskDelay(std::max<TickType_t>(1, pdMS_TO_TICKS(150)));
}

void CUC1638::_DC(bool on) {
    if (on) {
        // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_SET);
    } else {
        // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_RESET);
    }
}

void CUC1638::_CS(bool on) {
    if (!on) {
        // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
    } else {
        // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
    }
} 

void CUC1638::PowerUpSequence() {
    _WriteCommand(0xe1);//system reset
    _WriteData(0xe2);
    vTaskDelay(std::max<TickType_t>(1, pdMS_TO_TICKS(5)));

    _WriteCommand(0x04);//set column Address
    _WriteData(0x00);
    _WriteCommand(0x60);//set page Address
    _WriteCommand(0x70);

    //_WriteCommand(0x2f);// internal VLCD
    //_WriteCommand(0x26);// TC
    _WriteCommand(0xEb);//set bias=1/12
    _WriteCommand(0x81);//set vop
    _WriteData(60);//  pm=106 Set VLCD=15V

    _WriteCommand(0xb8);//屏蔽MTP
    _WriteData(0x00);

    
    _WriteCommand(0xA3);//set line rate  20klps
    _WriteCommand(0x95);  // PT0   1B P P
    // _WriteCommand(90);
    
    _WriteCommand(0xf1);   //set com end
    _WriteData(95);    //set com end   240*96
    
    _WriteCommand(0xC2);
    _WriteCommand(0x31);   //APC
    _WriteData(0X91);    // 1/0: sys_LRM_EN disable
    _WriteCommand(0xC4);//set lcd mapping control
    
    _WriteCommand(0xc9);
    _WriteData(0xad);  //  display 
    
    _WriteCommand(0xC2);//set lcd mapping control
}

void CUC1638::_WriteCommand(uint8_t cmd) {
    _DC(false);
    _CS(true);
    hspi2_.TxCpltCallback = [](SPI_HandleTypeDef* hspi) { xSemaphoreGiveFromISR(dmaSemHandle_, nullptr); };
    HAL_SPI_Transmit_IT(&hspi2_, &cmd, 1);
    xSemaphoreTake(dmaSemHandle_, portMAX_DELAY);
    _CS(false);
}

void CUC1638::_WriteData(uint8_t data) {
    _DC(true);
    _CS(true);
    hspi2_.TxCpltCallback = [](SPI_HandleTypeDef* hspi) { xSemaphoreGiveFromISR(dmaSemHandle_, nullptr); };
    HAL_SPI_Transmit_IT(&hspi2_, &data, 1);
    xSemaphoreTake(dmaSemHandle_, portMAX_DELAY);
    _CS(false);
}

// extern "C" void SPI2_IRQHandler(void) {
//     HAL_SPI_IRQHandler(&hspi2_);
// }
extern "C" void SPI3_IRQHandler(void) {
    HAL_SPI_IRQHandler(&hspi2_);
}

extern "C" void DMA1_Stream1_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_);
}

}

#endif
