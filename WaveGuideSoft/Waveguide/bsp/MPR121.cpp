#include "MPR121.hpp"
#include "stm32h7xx_hal.h"
#include "SystemHook.hpp"
#include <cstdio>
#include <algorithm>

#include "FreeRTOS.h"
#include "semphr.h"

namespace bsp {

static I2C_HandleTypeDef hi2c4_;
static SemaphoreHandle_t i2cSemHandle_{ nullptr };
static StaticSemaphore_t i2cSem_;

void CMPR121::Init(uint32_t dataRate) {
    dataRate_ = dataRate;
    modWheelFilter_.Init(dataRate);
    pitchBendFilter_.Init(dataRate);
    xFilter_.Init(dataRate);
    yFilter_.Init(dataRate);
    modWheelFilter_.SetTime(20.0f);
    pitchBendFilter_.SetTime(20.0f);
    xFilter_.SetTime(20.0f);
    yFilter_.SetTime(20.0f);
    
    // i2c init
    hi2c4_.Instance = I2C4;
    __HAL_RCC_I2C4_CLK_ENABLE();
    HAL_I2C_DeInit(&hi2c4_);

    hi2c4_.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c4_.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c4_.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c4_.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    hi2c4_.Init.OwnAddress1 = 0;
    hi2c4_.Init.OwnAddress2 = 0;
    hi2c4_.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c4_.Init.Timing = 0x00C0EAFF;
    HAL_I2C_Init(&hi2c4_);

    // i2c gpio init
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitTypeDef init;
    init.Pin = GPIO_PIN_12 | GPIO_PIN_13;
    init.Mode = GPIO_MODE_AF_OD;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    init.Alternate = GPIO_AF4_I2C4;
    HAL_GPIO_Init(GPIOD, &init);

    HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
    HAL_NVIC_SetPriority(I2C4_EV_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(I2C4_ER_IRQn);
    HAL_NVIC_SetPriority(I2C4_ER_IRQn, 10, 0);

    // semaphore init
    i2cSemHandle_ = xSemaphoreCreateBinaryStatic(&i2cSem_);

    PowerUp(touchPadAddress_);
    PowerUp(modWheelAdress_);
}

uint16_t CMPR121::_GetEleData(uint8_t address, int ele) {
    uint8_t LSB = ReadByte(address, 0x04 + ele * 2);
    uint8_t MSB = ReadByte(address, 0x04 + ele * 2 + 1);
    return ((MSB & 0b00000011) << 8) | LSB;
}

uint16_t CMPR121::_GetTounchData(uint8_t address) {
    uint8_t LSB = ReadByte(address, 0x00);
    uint8_t MSB = ReadByte(address, 0x01);
    return (MSB & 0x1f) << 8 | LSB;
}

static constexpr uint32_t kRemapTable[] {
    3, 4, 5, 7, 8, 10, 2, 1, 0, 11, 9, 6
};
void CMPR121::UpdateData() {
    // transfer
    touchData_ = _GetTounchData(touchPadAddress_);
    for (uint32_t i = 0; i < kNumEles; ++i) {
        eleData_[i] = _GetEleData(touchPadAddress_, i);
    }
    modWheelTouchData_ = _GetTounchData(modWheelAdress_);
    for (uint32_t i = 0; i < kNumEles; ++i) {
        modWheelEleData_[i] = _GetEleData(modWheelAdress_, i);
    }

    // process touchpad
    if (this->IsFingerPressed()) {
        // remap data
        for (uint32_t i = 0; i < kNumEles; ++i) {
            remapEleData_[i] = eleData_[kRemapTable[i]];
        }
        // x: 0-5
        {
            auto maxIt = std::min_element(remapEleData_, remapEleData_ + 6);
            auto idx = maxIt - remapEleData_;
            float pos = 0;
            if (idx == 0) {
                int32_t up = remapEleData_[1];
                int32_t down = remapEleData_[0] + remapEleData_[1];
                pos = 1.0f - static_cast<float>(up) / static_cast<float>(down);
            }
            else if (idx == 5) {
                int32_t up = remapEleData_[5];
                int32_t down = remapEleData_[4] + remapEleData_[5];
                pos = 4.0f + static_cast<float>(up) / static_cast<float>(down);
            }
            else {
                int32_t up = -remapEleData_[idx - 1] + remapEleData_[idx + 1];
                int32_t down = remapEleData_[idx - 1] + remapEleData_[idx] + remapEleData_[idx + 1];
                pos = idx - static_cast<float>(up) / static_cast<float>(down);
            }
            position_.fX = pos / 5.0f;
        }
        // y: 6-11
        {
            auto* yArrayBegin = remapEleData_ + 6;
            auto maxIt = std::min_element(yArrayBegin, remapEleData_ + 12);
            auto idx = maxIt - yArrayBegin;
            float pos = 0;
            if (idx == 0) {
                int32_t up = yArrayBegin[1];
                int32_t down = yArrayBegin[0] + yArrayBegin[1];
                pos = 1.0f - static_cast<float>(up) / static_cast<float>(down);
            }
            else if (idx == 5) {
                int32_t up = yArrayBegin[5];
                int32_t down = yArrayBegin[4] + yArrayBegin[5];
                pos = 4.0f + static_cast<float>(up) / static_cast<float>(down);
            }
            else {
                int32_t up = -yArrayBegin[idx - 1] + yArrayBegin[idx + 1];
                int32_t down = yArrayBegin[idx - 1] + yArrayBegin[idx] + yArrayBegin[idx + 1];
                pos = idx - static_cast<float>(up) / static_cast<float>(down);
            }
            position_.fY = pos / 5.0f;
        }
    }
    position_.fX = xFilter_.Process(position_.fX);
    position_.fY = yFilter_.Process(position_.fY);

    // process modwheel
    if (this->IsModWheelTouched()) {
        auto maxIt = std::min_element(modWheelEleData_, modWheelEleData_ + 6);
        auto idx = maxIt - modWheelEleData_;
        float pos = 0;
        if (idx == 0) {
            int32_t up = modWheelEleData_[1];
            int32_t down = modWheelEleData_[0] + modWheelEleData_[1];
            pos = 1.0f - static_cast<float>(up) / static_cast<float>(down);
        }
        else if (idx == 5) {
            int32_t up = modWheelEleData_[5];
            int32_t down = modWheelEleData_[4] + modWheelEleData_[5];
            pos = 4.0f + static_cast<float>(up) / static_cast<float>(down);
        }
        else {
            int32_t up = -modWheelEleData_[idx - 1] + modWheelEleData_[idx + 1];
            int32_t down = modWheelEleData_[idx - 1] + modWheelEleData_[idx] + modWheelEleData_[idx + 1];
            pos = idx - static_cast<float>(up) / static_cast<float>(down);
        }
        modWheelPos_ = 1.0f - pos / 5.0f;
    }
    else {
        modWheelPos_ = 0;
    }
    modWheelPos_ = modWheelFilter_.Process(modWheelPos_);

    if (this->IsPitchBendTouched()) {
        auto* yArrayBegin = modWheelEleData_ + 6;
        auto maxIt = std::min_element(yArrayBegin, modWheelEleData_ + 12);
        auto idx = maxIt - yArrayBegin;
        float pos = 0;
        if (idx == 0) {
            int32_t up = yArrayBegin[1];
            int32_t down = yArrayBegin[0] + yArrayBegin[1];
            pos = 1.0f - static_cast<float>(up) / static_cast<float>(down);
        }
        else if (idx == 5) {
            int32_t up = yArrayBegin[5];
            int32_t down = yArrayBegin[4] + yArrayBegin[5];
            pos = 4.0f + static_cast<float>(up) / static_cast<float>(down);
        }
        else {
            int32_t up = -yArrayBegin[idx - 1] + yArrayBegin[idx + 1];
            int32_t down = yArrayBegin[idx - 1] + yArrayBegin[idx] + yArrayBegin[idx + 1];
            pos = idx - static_cast<float>(up) / static_cast<float>(down);
        }
        pitchBendPos_ = pos / 5.0f;
    }
    else {
        pitchBendPos_ = 0.5f;
    }
    pitchBendPos_ = pitchBendFilter_.Process(pitchBendPos_);
}

void CMPR121::SetPositionFilterTime(float ms) {
    xFilter_.SetTime(ms);
    yFilter_.CopyCoeff(xFilter_);
}

void CMPR121::WriteByte(uint8_t address, uint8_t reg, uint8_t data) {
    hi2c4_.MemTxCpltCallback = [](I2C_HandleTypeDef* hi2c) { xSemaphoreGiveFromISR(i2cSemHandle_, nullptr); };
    HAL_I2C_Mem_Write_IT(&hi2c4_, address, reg, I2C_MEMADD_SIZE_8BIT, &data, 1);
    xSemaphoreTake(i2cSemHandle_, portMAX_DELAY);
}

static uint8_t data{};
uint8_t CMPR121::ReadByte(uint8_t address, uint8_t reg) {
    hi2c4_.MemRxCpltCallback = [](I2C_HandleTypeDef* hi2c) { xSemaphoreGiveFromISR(i2cSemHandle_, nullptr); };
    HAL_I2C_Mem_Read_IT(&hi2c4_, address, reg, I2C_MEMADD_SIZE_8BIT, &data, 1);
    xSemaphoreTake(i2cSemHandle_, portMAX_DELAY);
    return data;
}

uint16_t CMPR121::GetEleDataByNum(int num) const {
    return eleData_[kRemapTable[num]];
}

#define TouchThre 10//15//30//10
#define ReleaThre 8
// 8//25//8
void CMPR121::PowerUp(uint8_t address) {
    //Reset MPR121 if not reset correctly
    WriteByte(address, 0x80,0x63); //Soft reset
    WriteByte(address, 0x5E,0x00); //Stop mode

    //touch pad baseline filter
    //rising
    WriteByte(address, 0x2B,0x01); //0xFF// MAX HALF DELTA Rising
    WriteByte(address, 0x2C,0x01); //0xFF// NOISE HALF DELTA Rising
    WriteByte(address, 0x2D,0x00); // //0 NOISE COUNT LIMIT Rising
    WriteByte(address, 0x2E,0x00); // DELAY LIMIT Rising
    //falling
    WriteByte(address, 0x2F,0x01); // MAX HALF DELTA Falling
    WriteByte(address, 0x30,0x01); // NOISE HALF DELTA Falling
    WriteByte(address, 0x31,0xFF); // NOISE COUNT LIMIT Falling
    WriteByte(address, 0x32,0x02); // //2//DELAY LIMIT Falling
    //touched
    WriteByte(address, 0x33,0x00); // Noise half delta touched
    WriteByte(address, 0x34,0x00); // Noise counts touched
    WriteByte(address, 0x35,0x00); //Filter delay touched
    //Touch pad threshold
    WriteByte(address, 0x41,TouchThre); // ELE0 TOUCH THRESHOLD
    WriteByte(address, 0x42,ReleaThre); // ELE0 RELEASE THRESHOLD
    WriteByte(address, 0x43,TouchThre); // ELE1 TOUCH THRESHOLD
    WriteByte(address, 0x44,ReleaThre); // ELE1 RELEASE THRESHOLD
    WriteByte(address, 0x45,TouchThre); // ELE2 TOUCH THRESHOLD
    WriteByte(address, 0x46,ReleaThre); // ELE2 RELEASE THRESHOLD
    WriteByte(address, 0x47,TouchThre); // ELE3 TOUCH THRESHOLD
    WriteByte(address, 0x48,ReleaThre); // ELE3 RELEASE THRESHOLD
    WriteByte(address, 0x49,TouchThre); // ELE4 TOUCH THRESHOLD
    WriteByte(address, 0x4A,ReleaThre); // ELE4 RELEASE THRESHOLD
    WriteByte(address, 0x4B,TouchThre); // ELE5 TOUCH THRESHOLD
    WriteByte(address, 0x4C,ReleaThre); // ELE5 RELEASE THRESHOLD
    WriteByte(address, 0x4D,TouchThre); // ELE6 TOUCH THRESHOLD
    WriteByte(address, 0x4E,ReleaThre); // ELE6 RELEASE THRESHOLD
    WriteByte(address, 0x4F,TouchThre); // ELE7 TOUCH THRESHOLD
    WriteByte(address, 0x50,ReleaThre); // ELE7 RELEASE THRESHOLD
    WriteByte(address, 0x51,TouchThre); // ELE8 TOUCH THRESHOLD
    WriteByte(address, 0x52,ReleaThre); // ELE8 RELEASE THRESHOLD
    WriteByte(address, 0x53,TouchThre); // ELE9 TOUCH THRESHOLD
    WriteByte(address, 0x54,ReleaThre); // ELE9 RELEASE THRESHOLD
    WriteByte(address, 0x55,TouchThre); // ELE10 TOUCH THRESHOLD
    WriteByte(address, 0x56,ReleaThre); // ELE10 RELEASE THRESHOLD
    WriteByte(address, 0x57,TouchThre); // ELE11 TOUCH THRESHOLD
    WriteByte(address, 0x58,ReleaThre); // ELE11 RELEASE THRESHOLD

    //AFE configuration
    WriteByte(address, 0x5D,0x00);
    WriteByte(address, 0x5C,0xC0);
    //Auto configuration
    WriteByte(address, 0x7B,0xCB);
    WriteByte(address, 0x7D,0xE4);
    WriteByte(address, 0x7E,0x94);
    WriteByte(address, 0x7F,0xCD);
    WriteByte(address, 0x5E,0x0C); 
}

extern "C" void I2C4_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&hi2c4_);
}
    
extern "C" void I2C4_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&hi2c4_);
}

}
