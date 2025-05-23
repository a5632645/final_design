#pragma once
#include <cstdint>
#include "TimeKnob.hpp"
#include "../GuiDispatch.hpp"

namespace gui::internal {

class CReverb : public gui::CGuiObj {
public:
    void Draw(StyleDrawer& drawer) override;
    void OnSelect() override;
    void OnTimeTick(uint32_t msEscape) override;
private:
    void SetPageIdx(int8_t idx);

    int8_t selectIdx_ = 0;
    int8_t pageIdx_{};
    TimeKnob knobs_[12];
};

struct InternalReverb {
    inline static CReverb instance;
};

}

namespace gui {
static auto& Reverb = internal::InternalReverb::instance;
}