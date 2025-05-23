#pragma once
#include "../GuiDispatch.hpp"
#include "TimeKnob.hpp"

namespace gui::internal {

class CBowedDebug : public CGuiObj {
public:
    void Draw(StyleDrawer& drawer) override;
    void OnSelect() override;
    void OnTimeTick(uint32_t msEscape) override;
private:
    static constexpr uint32_t kNumKnob = 9;
    TimeKnob knobs_[kNumKnob];
    int32_t selectIdx_ = 0;
};

struct InternalBowedDebug {
    inline static CBowedDebug instance;
};

}

namespace gui {

static auto& BowedDebug = internal::InternalBowedDebug::instance;

}