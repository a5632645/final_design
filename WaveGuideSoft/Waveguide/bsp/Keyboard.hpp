#pragma once
#include <cstdint>
#include <span>
#include "dsp/OnePoleFilter.hpp"

namespace bsp {

class CKeyboard {
public:
    static constexpr uint32_t kNumKeys = 12;

    struct KeyTransferData {
        uint8_t adcValue;
        int8_t fingerPostion;
    };

    void Init(uint32_t dataRate);
    void UpdateData();
    void ProcessData();
    std::span<KeyTransferData> GetDatas();
    uint8_t GetPressure(uint8_t keyIndex) const;
    int8_t GetFingerPosition(uint8_t keyIndex) const;
    uint8_t GetAdcValue(uint8_t keyIndex) const;

    void SetADCUpValue(uint8_t value) { adcUpValue_ = value; }
    uint8_t GetADCUpValue() const { return adcUpValue_; }
    void SetADCDownValue(uint8_t value) { adcDownValue_ = value; }
    uint8_t GetADCDownValue() const { return adcDownValue_; }
    void SetKeyDownCallback(void(*callback)(uint8_t keyIdx)) { keyDownCallback_ = callback; }
    void SetKeyUpCallback(void(*callback)(uint8_t keyIdx)) { keyUpCallback_ = callback; }
    void SetMinAdcValue(uint8_t value) { adcMinValue_ = value; }
    uint8_t GetMinAdcValue() const { return adcMinValue_; }

    void SetMPEEnable(bool enable) { enableMPE_ = enable; }
    bool IsMPEEnabled() const { return enableMPE_; }

    void SetDataFilterAlphas(float alpha);
    float GetDataFilterAlphas() const { return dataFilterAlphas_; }
    uint32_t GetDataRate() const { return dataRate_; }
    bool IsKeyPressed(uint8_t idx) const;
private:
    void Keyboard_Read(uint8_t idx);
    void Keyboard_Write(uint8_t idx);

    float dataFilterAlphas_{ 0.9f };
    int8_t filteredPositions_[kNumKeys]{};
    uint8_t adcUpValue_{ 120 };
    uint8_t adcDownValue_{ 115 };
    uint16_t keyPressFlags_{};
    uint16_t lastKeyPressFlags_{};
    void(*keyDownCallback_)(uint8_t keyIdx){};
    void(*keyUpCallback_)(uint8_t keyIdx){};
    bool enableMPE_{ true };
    uint32_t dataRate_{};
    uint8_t adcMinValue_{ 50 };
};

namespace internal {
struct InternalKeyboard {
    inline static CKeyboard instance;
};
}

inline static auto& Keyboard = internal::InternalKeyboard::instance;

}