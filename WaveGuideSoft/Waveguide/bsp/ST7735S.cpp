#include "ST7735S.hpp"
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

namespace bsp {

static SPI_HandleTypeDef hspi2_;
static DMA_HandleTypeDef hdma_;
MEM_DMA_SRAMD1 static OLEDRGBColor displayBuffer[OLEDDisplay::kBufferSize];
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
    hspi2_.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
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
    _WriteCommand(0x2A);	// set column(x) address
	WriteAddress(0, display.kWidth);
	_WriteCommand(0x2B);	// set Page(y) address
	WriteAddress(0, display.kHeight);
	_WriteCommand(0x2C);	//	Memory Write
    _DC(true);
    _CS(true);
    hspi2_.TxCpltCallback = [](SPI_HandleTypeDef* hspi) { xSemaphoreGiveFromISR(dmaSemHandle_, nullptr); };
    HAL_SPI_Transmit_DMA(&hspi2_, reinterpret_cast<const uint8_t*>(display.getDisplayBuffer()), display.kBufferSize * sizeof(OLEDRGBColor));
    xSemaphoreTake(dmaSemHandle_, portMAX_DELAY);
    _CS(false);
}

void CUC1638::SetBKlight(bool on) {
    if (on) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);
    }
}

void CUC1638::SetMirror(bool x, bool y) {
    uint8_t xv = x ? 0b01000000 : 0;
    uint8_t yv = y ? 0b10000000 : 0;
    _WriteCommand(0x36);	//Memory Data Access Control 
    _WriteData(0x28 | xv | yv);	//BGR color filter panel
}

void CUC1638::HardReset() {
    // ground is reset
    HAL_GPIO_WritePin(LCD_RESET_GPIO, LCD_RESET_PIN, GPIO_PIN_RESET);
    vTaskDelay(std::max<TickType_t>(1, pdMS_TO_TICKS(10)));
    HAL_GPIO_WritePin(LCD_RESET_GPIO, LCD_RESET_PIN, GPIO_PIN_SET);
    vTaskDelay(std::max<TickType_t>(1, pdMS_TO_TICKS(150)));
}

void CUC1638::_DC(bool on) {
    if (on) {
        HAL_GPIO_WritePin(LCD_DC_GPIO, LCD_DC_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LCD_DC_GPIO, LCD_DC_PIN, GPIO_PIN_RESET);
    }
}

void CUC1638::_CS(bool on) {
    if (on) {
        HAL_GPIO_WritePin(LCD_CS_GPIO, LCD_CS_PIN, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(LCD_CS_GPIO, LCD_CS_PIN, GPIO_PIN_SET);
    }
} 

void CUC1638::PowerUpSequence() {
    _WriteCommand(0x01);	//Software Reset 
    vTaskDelay(pdMS_TO_TICKS(150));

    _WriteCommand(0x11);	//Sleep Out
    vTaskDelay(pdMS_TO_TICKS(255));

    {
        _WriteCommand(0xB1);	//Frame Rate Control (In normal mode/ Full colors)
        uint8_t seq[] {
            0x01,
            0x2C,
            0x2D,
        };
        WriteDataSpan(seq);
    }

    {
        _WriteCommand(0xB2);	//Frame Rate Control (In Idle mode/ 8-colors) 
        uint8_t seq[] {
            0x01,
            0x2C,
            0x2D,
        };
        WriteDataSpan(seq);
    }

    {
        _WriteCommand(0xB3);	//Frame Rate Control (In Partial mode/ full colors) 
        uint8_t seq[] {
            0x01,
            0x2C,
            0x2D,
            0x01,
            0x2C,
            0x2D,
        };
        WriteDataSpan(seq);
    }

    _WriteCommand(0xB4);	//Display Inversion Control
    _WriteData(0x07);

    {
        _WriteCommand(0xC0);	//Power Control 1
        uint8_t seq[] {
            0xA2,
            0x02,
            0x84,
        };
        WriteDataSpan(seq);
    }

    _WriteCommand(0xC1);	//Power Control 2
    _WriteData(0xC5);

    {
        _WriteCommand(0xC2);	//Power Control 3 (in Normal mode/ Full colors) 
        uint8_t seq[] {
            0x0A,
            0x00,
        };
        WriteDataSpan(seq);
    }

    {
        _WriteCommand(0xC3);	//Power Control 4 (in Idle mode/ 8-colors)
        uint8_t seq[] {
            0x8A,
            0x2A,
        };
        WriteDataSpan(seq);
    }

    {
        _WriteCommand(0xC4);	//Power Control 5 (in Partial mode/ full-colors)
        uint8_t seq[] {
            0x8A,
            0xEE,
        };
        WriteDataSpan(seq);
    }

    _WriteCommand(0xC5);	//VCOM Control 1
    _WriteData(0x0E);

    _WriteCommand(0x20);	//Display Inversion Off 

    // _WriteCommand(0x36);	//Memory Data Access Control 
    // _WriteData(0x28);	//BGR color filter panel
    // // _WriteData(0xC0);	//RGB color filter panel
    SetMirror(true, false);

    _WriteCommand(0x3A);	//Interface Pixel Format
    _WriteData(0x05);	//16-bit/pixel 65K-Colors(RGB 5-6-5-bit Input)

    // _WriteCommand(0x2A);	//Column Address Set
    // _WriteData(0x00);
    // _WriteData(0x02);
    // _WriteData(0x00);
    // _WriteData(0x81);

    // _WriteCommand(0x2B);	//Row Address Set
    // _WriteData(0x00);
    // _WriteData(0x01);
    // _WriteData(0x00);
    // _WriteData(0xA0);

    // _WriteCommand(0x21);	//Display Inversion On

    {
        _WriteCommand(0xE0);	//Gamma (‘+’polarity) Correction Characteristics Setting
        uint8_t seq[] {
            0x02,
            0x1C,
            0x07,
            0x12,
            0x37,
            0x32,
            0x29,
            0x2D,
            0x29,
            0x25,
            0x2B,
            0x39,
            0x00,
            0x01,
            0x03,
            0x10,
        };
        WriteDataSpan(seq);
    }

    {
        _WriteCommand(0xE1);	//Gamma ‘-’polarity Correction Characteristics Setting
        uint8_t seq[] {
            0x03,
            0x1D,
            0x07,
            0x06,
            0x2E,
            0x2C,
            0x29,
            0x2D,
            0x2E,
            0x2E,
            0x37,
            0x3F,
            0x00,
            0x00,
            0x02,
            0x10,
        };
        WriteDataSpan(seq);
    }

    _WriteCommand(0x13);	//Normal Display Mode On
    vTaskDelay(pdMS_TO_TICKS(10));

    _WriteCommand(0x29);	//Display On
    vTaskDelay(pdMS_TO_TICKS(100));
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

extern "C" void SPI3_IRQHandler(void) {
    HAL_SPI_IRQHandler(&hspi2_);
}

extern "C" void DMA1_Stream1_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_);
}

void CUC1638::WriteAddress(uint16_t address1, uint16_t address2) {
    uint8_t seq[] {
        (address1 >> 8 & 0xff),
        (address1 & 0xff),
        (address2 >> 8 & 0xff),
        (address2 & 0xff),
    };
    _DC(true);
    _CS(true);
    hspi2_.TxCpltCallback = [](SPI_HandleTypeDef* hspi) { xSemaphoreGiveFromISR(dmaSemHandle_, nullptr); };
    HAL_SPI_Transmit_IT(&hspi2_, seq, sizeof(seq));
    xSemaphoreTake(dmaSemHandle_, portMAX_DELAY);
    _CS(false);
}

void CUC1638::WriteDataSpan(std::span<uint8_t> data) {
    _DC(true);
    _CS(true);
    hspi2_.TxCpltCallback = [](SPI_HandleTypeDef* hspi) { xSemaphoreGiveFromISR(dmaSemHandle_, nullptr); };
    HAL_SPI_Transmit_IT(&hspi2_, data.data(), data.size_bytes());
    xSemaphoreTake(dmaSemHandle_, portMAX_DELAY);
    _CS(false);
}
}
