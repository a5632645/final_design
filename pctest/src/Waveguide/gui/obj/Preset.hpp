#pragma once
#include "../GuiDispatch.hpp"

namespace gui::internal {

class CPreset : public CGuiObj {
public:
    static constexpr int32_t kNumCanDisplay = 8;

    void Draw(StyleDrawer& display) override;
    void OnSelect() override;
private:
    void EditText(char* ptr, int32_t length);

    int32_t listPos_{};
    int32_t listTopPos_{};
    int32_t numTotalListItems_{};

    bool isSaving_{false};
    char editBuffer_[12]{};
    int32_t editPos_{};
    int32_t editBufferLength_{};
};

struct InternalPreset {
    inline static CPreset instance;
};

}

namespace gui {

static auto& Preset = internal::InternalPreset::instance;

}