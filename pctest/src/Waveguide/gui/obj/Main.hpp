#pragma once
#include "../GuiDispatch.hpp"

namespace gui::internal {

struct CMain : public CGuiObj {
    void Draw(StyleDrawer& display) override;
    void OnSelect() override;

    void NoteOn(uint8_t i) { keyPressState |= (1 << i); GuiDispatch.SetNeedUpdate(); }
    void NoteOff(uint8_t i) { keyPressState &= ~(1 << i); GuiDispatch.SetNeedUpdate(); }
    void SetOctave(int i) { octave = i; GuiDispatch.SetNeedUpdate(); }
    void AddOctave(int i);

    uint16_t keyPressState = 0;
    int octave = 4;
    int selectIdx = 0;
};

struct InternalMain {
    inline static CMain instance;
};

}

namespace gui {

static auto& Main = internal::InternalMain::instance;

}
