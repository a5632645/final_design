#include "MidiManager.hpp"
#include <algorithm>
#include "FreeRTOS.h"
#include "bsp/Keyboard.hpp"

void CMidiManager::Init(uint32_t dataUpdateRate) {
    std::fill_n(channelTable, 128, kInvalidChannel);
}

void CMidiManager::NoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    if (note < 32) {
        reg0 |= kOne << note;
    }
    else if (note < 64) {
        reg1 |= kOne << (note - 32);
    }
    else if (note < 96) {
        reg2 |= kOne << (note - 64);
    }
    else {
        reg3 |= kOne << (note - 96);
    }
    velocityTable[note] = velocity;
    channelTable[note] = channel;
}

uint8_t CMidiManager::NoteOn(uint8_t note, uint8_t velocity) {
    auto channel = note % 12; // 0~14
    if (!bsp::Keyboard.IsMPEEnabled()) {
        channel = 0;
    }
    NoteOn(channel, note, velocity);
    return channel;
}

void CMidiManager::NoteOff(uint8_t channel, uint8_t note) {
    if (note < 32) {
        reg0 &= ~(kOne << note);
    }
    else if (note < 64) {
        reg1 &= ~(kOne << (note - 32));
    }
    else if (note < 96) {
        reg2 &= ~(kOne << (note - 64));
    }
    else {
        reg3 &= ~(kOne << (note - 96));
    }
    velocityTable[note] = 0;
    channelTable[note] = kInvalidChannel;
}

uint8_t CMidiManager::NoteOff(uint8_t note) {
    auto channel = note % 12; // 1~12
    if (!bsp::Keyboard.IsMPEEnabled()) {
        channel = 0;
    }
    NoteOff(channel, note);
    return channel;
}

void CMidiManager::BackupState() {
    diff0 = reg0 ^ lastReg0;
    diff1 = reg1 ^ lastReg1;
    diff2 = reg2 ^ lastReg2;
    diff3 = reg3 ^ lastReg3;
    lastReg0 = reg0;
    lastReg1 = reg1;
    lastReg2 = reg2;
    lastReg3 = reg3;
}

void CMidiManager::Lock() {
    portENTER_CRITICAL();
}

void CMidiManager::Unlock() {
    portEXIT_CRITICAL();
}

uint8_t CMidiManager::GetChannelOfNote(uint8_t note) const {
    return channelTable[note];
}

uint8_t CMidiManager::GetVelocity(uint8_t note) const {
    return velocityTable[note];
}

void CMidiManager::SetTouchSliderPos(uint8_t idx, int8_t val) {
    touchSliders_[idx] = val / 127.0f;
}

void CMidiManager::SetTouchSliderPos(uint8_t idx, float val) {
    touchSliders_[idx] = val;
}

float CMidiManager::GetTouchSliderValue(uint8_t channel) {
    return touchSliders_[channel];
}

CMidiManager MidiManager;