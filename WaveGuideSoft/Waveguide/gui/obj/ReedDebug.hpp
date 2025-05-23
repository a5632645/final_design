#pragma once
#include "../GuiDispatch.hpp"
#include "TimeKnob.hpp"

namespace gui::internal {

class CReedDebug : public CGuiObj {
public:
    void Draw(StyleDrawer& drawer) override;
    void OnSelect() override;
    void OnTimeTick(uint32_t msEscape) override;
private:
    static constexpr uint32_t kNumKnob = 4;
    TimeKnob knobs_[kNumKnob];
    int32_t selectIdx_ = 0;
};

struct InternalReedDebug {
    inline static CReedDebug instance;
};

}

namespace gui {

static auto& ReedDebug = internal::InternalReedDebug::instance;

}