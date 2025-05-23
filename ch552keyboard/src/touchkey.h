#pragma once
#include "ch554.h"
#include "stdint.h"

inline void TouchKey_Fast(void) {
    TKEY_CTRL &= bTKC_2MS;
}
inline void TouchKey_Slow(void) {
    TKEY_CTRL |= bTKC_2MS;
}

enum TouchKeyChannel { TK_P10 = 1, TK_P11, TK_P14, TK_P15, TK_P16, TK_P17 };
inline void TouchKey_Channel(enum TouchKeyChannel channel) {
    uint8_t reg = TKEY_CTRL & 0xf8;
    TKEY_CTRL = reg | channel;
}
inline void TouchKey_Disable(void) {
    TKEY_CTRL &= 0x7;
}

inline __bit TouchKey_Ready(void) {
    return TKEY_CTRL & bTKC_IF;
}

inline uint8_t TouchKey_DataValid(void) {
    return TKEY_DATH & bTKD_CHG;
}

inline uint16_t TouchKey_PollRead(enum TouchKeyChannel channel) {
    TouchKey_Channel(channel);
    while (TouchKey_Ready() == 0) {}
    uint16_t touchReg = TKEY_DAT;
    return touchReg;
}

inline void TouchKey_EnableIRQ(void) {
    IE_TKEY = 1;
}

inline void TouchKey_DisableIRQ(void) {
    IE_TKEY = 0;
}