#pragma once
#include "../GuiDispatch.hpp"

namespace gui {

class CSettings : public CGuiObj {
public:
    void Draw(StyleDrawer& drawer) override;
    void OnSelect() override;
    void OnTimeTick(uint32_t msEscape) override;
private:
    void SetPageIdx(int8_t idx);
    
    int8_t selectIdx = 0;
    int8_t pageIdx_ = 0;
    bool isBkLightOn_{ true };
};

namespace internal {
struct InternalSettings {
    inline static CSettings instance;
};
}

static CSettings& Settings = internal::InternalSettings::instance;

}