#include "USBMidi.hpp"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_desc.h"
extern "C" {
    #include "usbd_midi.h"  
}

#include "SystemHook.hpp"

#include "FreeRTOS.h"
#include "semphr.h"

#include "bsp/DebugIO.hpp"

namespace bsp {

static USBD_HandleTypeDef USBD_Device;

// RX
static constexpr uint32_t kEventBufferSize = 128;
static MidiEvent midiRxBuffer_[kEventBufferSize];
static uint32_t numRx_{};
static void(*midiRxCallback_)(std::span<const MidiEvent>){};

// tx
static constexpr uint32_t kTxBufferSize = 128;
static constexpr uint32_t kUsbTxSize = MIDI_EPIN_SIZE;
static MidiEvent midiTxBuffer_[kTxBufferSize];
static uint32_t numMidiTx_{};
static StaticSemaphore_t txCompleteSem_;
static SemaphoreHandle_t txCompleteSemHandle_{};

void CUSBMidi::Init() {
    HAL_PWREx_EnableUSBVoltageDetector();

    /* Init Device Library */
    auto res = USBD_Init(&USBD_Device, &HID_Desc, 0);
    if (res != USBD_OK) {
        DEVICE_ERROR_CODE("USB", "USBD_Init failed", res);
    }

    /* Add Supported Class */
    res = USBD_RegisterClass(&USBD_Device, &USBD_MIDI);
    if (res != USBD_OK) {
        DEVICE_ERROR_CODE("USB", "USBD_RegisterClass failed", res);
    }

    /* Start Device Process */
    res = USBD_Start(&USBD_Device);
    if (res != USBD_OK) {
        DEVICE_ERROR_CODE("USB", "USBD_Start failed", res);
    }

    txCompleteSemHandle_ = xSemaphoreCreateBinaryStatic(&txCompleteSem_);
}

void CUSBMidi::SendData() {
    constexpr auto maxEvents = kUsbTxSize / sizeof(MidiEvent);
    if (numMidiTx_ > maxEvents) {
        USBD_MIDI_SendReport(&USBD_Device, reinterpret_cast<uint8_t*>(&midiTxBuffer_[0]), maxEvents * sizeof(MidiEvent));
        numMidiTx_ -= maxEvents;
        
    }
    else if (numMidiTx_ != 0) {
        USBD_MIDI_SendReport(&USBD_Device, reinterpret_cast<uint8_t*>(&midiTxBuffer_[0]), numMidiTx_ * sizeof(MidiEvent));
        numMidiTx_ = 0;
    }
    xSemaphoreTake(txCompleteSemHandle_, portMAX_DELAY);
}

void CUSBMidi::SetMidiReciveIRQ(void (*callback)(std::span<const MidiEvent>)) {
    midiRxCallback_ = callback;
}

void CUSBMidi::NotifySend(bool irq) {
    if (irq) {
        xSemaphoreGiveFromISR(txCompleteSemHandle_, nullptr);
    }
    else {
        xSemaphoreGive(txCompleteSemHandle_);
    }
}

void CUSBMidi::Write(MidiEvent e) {
    if (numMidiTx_ < kTxBufferSize) {
        midiTxBuffer_[numMidiTx_++] = e;
    }
}

void CUSBMidi::WriteNoteOn(uint8_t ch, uint8_t note, uint8_t vel) {
    MidiEvent e{};
    e.cableNumber = 0;
    e.codeIndexNumber = 0x9;
    e.data1 = 0x90 | ch;
    e.data2 = note;
    e.data3 = vel;
    Write(e);
}

void CUSBMidi::WriteNoteOff(uint8_t ch, uint8_t note, uint8_t vel) {
    MidiEvent e{};
    e.cableNumber = 0;
    e.codeIndexNumber = 0x8;
    e.data1 = 0x80 | ch;
    e.data2 = note;
    e.data3 = vel;
    Write(e);
}

void CUSBMidi::WriteCC(uint8_t ch, uint8_t ccNo, uint8_t ccVal) {
    MidiEvent e{};
    e.cableNumber = 0;
    e.codeIndexNumber = 0xb;
    e.data1 = 0xb0 | ch;
    e.data2 = ccNo;
    e.data3 = ccVal;
    Write(e);
}

void CUSBMidi::WritePitchBend(uint8_t ch, uint16_t val) {
    MidiEvent e{};
    e.cableNumber = 0;
    e.codeIndexNumber = 0xe;
    e.data1 = 0xe0 | ch;
    e.data2 = val & 0x7f;
    e.data3 = (val >> 7) & 0x7f;
    Write(e);
}

void CUSBMidi::WritePitchBendScaled(uint8_t ch, int8_t val) {
    int16_t v = 8192 + val * 64;
    WritePitchBend(ch, v);
}

void CUSBMidi::WriteChannelPressure(uint8_t ch, uint8_t val) {
    MidiEvent e{};
    e.cableNumber = 0;
    e.codeIndexNumber = 0xd;
    e.data1 = 0xd0 | ch;
    e.data2 = val;
    Write(e);
}

void CUSBMidi::WritePolyAfterTouch(uint8_t ch, uint8_t note, uint8_t val) {
    MidiEvent e{};
    e.cableNumber = 0;
    e.codeIndexNumber = 0xa;
    e.data1 = 0xa0 | ch;
    e.data2 = note;
    e.data3 = val;
    Write(e);
}

void CUSBMidi::MPEwriteNoteOn(uint8_t note, uint8_t vel) {
    auto ch = note % 12 + 1;
    WriteNoteOn(ch, note, vel);
}

void CUSBMidi::MPEwriteNoteOff(uint8_t note, uint8_t vel) {
    auto ch = note % 12 + 1;
    WriteNoteOff(ch, note, vel);
}

void CUSBMidi::MPEwritePitchBendScaled(uint8_t note, int8_t val) {
    auto ch = note % 12 + 1;
    WritePitchBendScaled(ch, val);
}

// --------------------------------------------------------------------------------
// IRQ
// --------------------------------------------------------------------------------
extern "C" void USBD_MIDI_DataInHandler(uint8_t* usb_rx_buffer, uint8_t usb_rx_buffer_length) {
    while (usb_rx_buffer_length && *usb_rx_buffer != 0x00) {
        uint32_t v = (usb_rx_buffer[3] << 24) | (usb_rx_buffer[2] << 16) | (usb_rx_buffer[1] << 8) | usb_rx_buffer[0];
        memcpy(&midiRxBuffer_[numRx_++], &v, 4);

        usb_rx_buffer += 4;
        usb_rx_buffer_length -= 4;
        numRx_ %= kEventBufferSize;
    }
    if (midiRxCallback_) {
        midiRxCallback_(std::span{midiRxBuffer_, numRx_});
    }
    numRx_ = 0;
}

extern "C" void USBD_MIDI_DataOutHandler() {
    xSemaphoreGiveFromISR(txCompleteSemHandle_, nullptr);
}

}