#pragma once
#include "../GuiDispatch.hpp"

namespace gui {

struct CMain : public CGuiObj {
    void Draw(StyleDrawer& display) override;
    void OnSelect() override;

    void NoteOn(uint8_t i) { keyPressState |= (1 << (i % 12)); Redraw(); }
    void NoteOff(uint8_t i) { keyPressState &= ~(1 << (i % 12)); Redraw(); }
    void SetOctave(int i) {
        octave = std::clamp(i, 2, 9);
        Redraw();
    }
    void SetDacTaskMs(uint32_t ms);

    uint16_t keyPressState = 0;
    int octave = 4;
    int8_t selectIdx_ = 0;
    uint32_t dacTaskTicks_ = 0;
};

namespace internal {
struct InternalMain {
    inline static CMain instance;
};
}

static CMain& Main = internal::InternalMain::instance;

} // namespace gui
