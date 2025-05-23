#pragma once
#include <span>
#include <cstdint>

namespace bsp {

struct MidiEvent {
    enum class Type {
        kNoteOn = 0x9,
        kNoteOff = 0x8,
        kPitchBend = 0xe,
        kPressure = 0xd,
        kCC = 0xb
    };

    uint8_t codeIndexNumber : 4;
    uint8_t cableNumber : 4;
    uint8_t data1;
    uint8_t data2;
    uint8_t data3;

    bool IsNoteOn() const { return codeIndexNumber == 9; }
    bool IsNoteOff() const { return codeIndexNumber == 8; }
    uint8_t GetNote() const { return data2; }
    uint8_t GetChannel() const { return data1 & 0xf; }
    uint8_t GetVelocity() const { return data3; }

    Type GetType() const { return static_cast<Type>(codeIndexNumber); }

    bool IsPitchBend() const { return codeIndexNumber == 0xe; }
    uint16_t GetPitchBend() const { return (data2 & 0x7f) + ((data1 & 0x7f) << 7); }
};

class CUSBMidi {
public:
    void Init();
    void SendData();
    void SetMidiReciveIRQ(void(*callback)(std::span<const MidiEvent>));
    void NotifySend(bool irq);

    // write
    void Write(MidiEvent e);
    void WriteNoteOn(uint8_t ch, uint8_t note, uint8_t vel);
    void WriteNoteOff(uint8_t ch, uint8_t note, uint8_t vel);
    void WriteCC(uint8_t ch, uint8_t ccNo, uint8_t ccVal);
    void WritePitchBend(uint8_t ch, uint16_t val);
    void WritePitchBendScaled(uint8_t ch, int8_t val);
    void WriteChannelPressure(uint8_t ch, uint8_t val);
    void WritePolyAfterTouch(uint8_t ch, uint8_t note, uint8_t val);

    // MPE
    void MPEwriteNoteOn(uint8_t note, uint8_t vel);
    void MPEwriteNoteOff(uint8_t note, uint8_t vel);
    void MPEwritePitchBendScaled(uint8_t note, int8_t val);
};

namespace internal {
struct InternalUSBMIDI {
    inline static CUSBMidi instance;
};
}

inline static auto& USBMidi = internal::InternalUSBMIDI::instance;

}