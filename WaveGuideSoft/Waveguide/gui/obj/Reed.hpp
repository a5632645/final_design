#pragma once
#include "../GuiDispatch.hpp"
#include "TimeKnob.hpp"

namespace gui::internal {

struct CReed : public CGuiObj {
    void Draw(StyleDrawer& display) override;
    void OnSelect() override;
    void OnTimeTick(uint32_t msEscape) override;

    void SetPageIdx(int8_t idx);
private:
    static constexpr uint32_t kNumKnob = 12;
    TimeKnob knobs_[kNumKnob];
    int8_t selectIdx_ = 0;
    int8_t pageIdx_{};
    int8_t totalKnobs_{};
};

struct InternalReed {
    inline static CReed instance;
};

}

namespace gui {

static auto& Reed = internal::InternalReed::instance;

}
