#include "MidiManager.hpp"
#include <algorithm>

void CMidiManager::Init(uint32_t dataUpdateRate) {
    std::fill_n(channelTable, 128, kInvalidChannel);
    std::fill_n(lastChannelTable, 128, kInvalidChannel);
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
    std::copy_n(velocityTable, 128, lastVelocityTable);
    std::copy_n(channelTable, 128, lastChannelTable);
}

void CMidiManager::Lock() {
}

void CMidiManager::Unlock() {
}

uint8_t CMidiManager::GetLastChannelOfNote(uint8_t note) const {
    return lastChannelTable[note];
}

uint8_t CMidiManager::GetChannelOfNote(uint8_t note) const {
    return channelTable[note];
}

uint8_t CMidiManager::GetLastVelocity(uint8_t note) const {
    return lastVelocityTable[note];
}

uint8_t CMidiManager::GetVelocity(uint8_t note) const {
    return velocityTable[note];
}

void CMidiManager::SetPitchBend(uint8_t channel, uint8_t msb, uint8_t lsb) {
    auto pb = (lsb & 0x7f) + ((msb & 0x7f) << 7);
    SetTouchSliderPos(channel, (pb - 8192.0f) / 8192.0f);
}

void CMidiManager::SetPitchBend(uint8_t channel, uint8_t scaled) {
    SetTouchSliderPos(channel, (scaled - 128) / 127.0f);
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