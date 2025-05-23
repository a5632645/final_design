#pragma once
#include <cstdint>
#include "dsp/ExpSmoother2.hpp"

namespace bsp {

class CMPR121 {
public:
    static constexpr uint32_t kNumEles = 12;

    struct Position {
        float fX; // 0~1
        float fY; // 0~1
    };

    void Init(uint32_t dataRate);
    uint16_t GetEleData(int ele) const { return eleData_[ele]; }
    uint16_t GetEleDataByNum(int num) const;
    uint16_t GetTounchData() const { return touchData_; }
    bool IsFingerPressed() const { return touchData_ != 0; }
    const Position& GetPosition() const { return position_; }
    void UpdateData();
    void SetPositionFilterTime(float ms);
    float GetPositionFilterTime() const { return xFilter_.GetTime(); }

    bool IsModWheelTouched() const { return modWheelTouchData_ & 0b111111; }
    bool IsPitchBendTouched() const { return modWheelTouchData_ & 0b111111000000; }
    /**
     * @brief 
     * @return 0~1 
     */
    float GetPitchBendPos() const { return pitchBendPos_; }
    /**
     * @brief 
     * @return 0~1 
     */
    float GetModWheelPos() const { return modWheelPos_; }
    void SetModWheelFilter(float ms) { modWheelFilter_.SetTime(ms); }
    void SetPitchBendFilter(float ms) { pitchBendFilter_.SetTime(ms); }
    float GetModWheelFilterTime() const { return modWheelFilter_.GetTime(); }
    float GetPitchBendFilterTime() const { return pitchBendFilter_.GetTime(); }
private:
    uint16_t _GetEleData(uint8_t address, int ele);
    uint16_t _GetTounchData(uint8_t address);
    void PowerUp(uint8_t address);
    void WriteByte(uint8_t address, uint8_t reg, uint8_t data);
    uint8_t ReadByte(uint8_t address, uint8_t reg);

    uint16_t touchData_{};
    uint16_t eleData_[kNumEles]{};
    uint16_t modWheelTouchData_{};
    uint16_t modWheelEleData_[kNumEles]{};
    uint16_t remapEleData_[kNumEles]{};
    dsp::ExpSmoother2 xFilter_;
    dsp::ExpSmoother2 yFilter_;
    Position position_{};
    uint8_t touchPadAddress_ = 0x5a << 1;
    uint8_t modWheelAdress_ = 0x5b << 1;

    float modWheelPos_{ 0.0f };
    float pitchBendPos_{ 0.0f };
    dsp::ExpSmoother2 modWheelFilter_;
    dsp::ExpSmoother2 pitchBendFilter_;
    uint32_t dataRate_{};
};

namespace internal {
struct InternalMPR121 {
    inline static CMPR121 instance;
};
}

inline static CMPR121& MPR121 = internal::InternalMPR121::instance;

}