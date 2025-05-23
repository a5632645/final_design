#pragma once
#include <cstdint>
#include "oled/OLEDDisplay.h"

#if 0

namespace bsp {

class CUC1638 {
public:
    void Init();
    void UpdateScreen();

    void SetBKlight(bool on);
    void SetMirror(bool x, bool y);

    void HardReset();
    void PowerUpSequence();

    void _WriteCommand(uint8_t cmd);
    void _WriteData(uint8_t data);
    
    OLEDDisplay display;
private:
    void _DC(bool on);
    void _CS(bool on);
};

namespace internal {
struct InternalUC1638 {
    inline static CUC1638 instance;
};
}

inline static auto& UC1638 = internal::InternalUC1638::instance;

}
#else
#include "ST7735S.hpp"
#endif