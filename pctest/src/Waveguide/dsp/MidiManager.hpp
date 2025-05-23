#pragma once
#include <cstdint>

class CMidiManager {
public:
    static constexpr uint32_t kOne = 1;
    static constexpr uint8_t kInvalidChannel = 16;

    void Init(uint32_t dataUpdateRate);
    void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    [[nodiscard]] uint8_t NoteOn(uint8_t note, uint8_t velocity);
    void NoteOff(uint8_t channel, uint8_t note);
    [[nodiscard]] uint8_t NoteOff(uint8_t note);
    void BackupState();
    void Lock();
    void Unlock();
    uint8_t GetLastChannelOfNote(uint8_t note) const;
    uint8_t GetChannelOfNote(uint8_t note) const;
    uint8_t GetLastVelocity(uint8_t note) const;
    uint8_t GetVelocity(uint8_t note) const;

    uint32_t GetDiff0() const { return diff0; };
    uint32_t GetDiff1() const { return diff1; };
    uint32_t GetDiff2() const { return diff2; };
    uint32_t GetDiff3() const { return diff3; };

    void SetPressure(uint8_t channel, uint8_t pressure) { pressure_[channel] = pressure / 127.0f; }
    float GetPressure(uint8_t channel) const { return pressure_[channel] / 127.0f; }
    void SetCC(uint8_t channel, uint8_t cc, uint8_t value) { mpeCC[channel].cc[cc] = value; }
    uint8_t GetCC(uint8_t channel, uint8_t cc) const { return mpeCC[channel].cc[cc]; }
    void SetPitchBend(uint8_t channel, uint8_t msb, uint8_t lsb);
    void SetPitchBend(uint8_t channel, uint8_t scaled);
    // -1.0 to 1.0
    float GetPitchBend(uint8_t channel) const { return touchSliders_[channel]; }

    // touchpad
    void SetTouchPad(float x, float y) { touchPadX_ = x; touchPadY_ = y; }
    void SetIsTouched(bool isTouched) { isTouched_ = isTouched; }
    /**
     * @brief 
     * @return 0.0 to 1.0 
     */
    float GetTouchPadX() const { return touchPadX_; }

    /**
     * @brief 
     * @return 0.0 to 1.0
     */
    float GetTouchPadY() const { return touchPadY_; }

    // touchsliders
    void SetTouchSliderPos(uint8_t idx, int8_t val);
    void SetTouchSliderPos(uint8_t idx, float val);
    /**
     * @brief 
     * @param channel value between 1 to 12
     * @return -1.0 to 1.0
     */
    float GetTouchSliderValue(uint8_t channel);
private:
    uint32_t reg0{};
    uint32_t reg1{};
    uint32_t reg2{};
    uint32_t reg3{};
    uint32_t diff0{};
    uint32_t diff1{};
    uint32_t diff2{};
    uint32_t diff3{};
    uint32_t lastReg0{};
    uint32_t lastReg1{};
    uint32_t lastReg2{};
    uint32_t lastReg3{};
    uint8_t velocityTable[128]{};
    uint8_t lastVelocityTable[128]{};
    
    // mpe
    uint8_t channelTable[128]{};
    uint8_t lastChannelTable[128]{};
    struct CCList {
        uint8_t cc[128]{};
    };
    CCList mpeCC[16]{};
    
    // touchpad
    bool isTouched_{};
    float touchPadX_{};
    float touchPadY_{};

    // touch slider
    float touchSliders_[16]{};
    float pressure_[16]{};
};

extern CMidiManager MidiManager;