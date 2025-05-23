#pragma once
#include "../GuiDispatch.hpp"
#include <span>
#include <array>

namespace gui::internal {

class CScope : public CGuiObj {
public:
    void Draw(StyleDrawer& display) override;
    void OnSelect() override;
    void Push(std::span<float> block);
private:
    std::array<float, OLEDDisplay::kWidth - 2 - 8> scopeBuffer_{};
    int32_t sampleInterval_{ 16 };
    uint32_t writePos_{};
    float sclae_{ 3.981071705534972f };
    float dBSclae_{ 12.0f };
    int8_t selectIdx_{};
};

struct InternalScope {
    inline static CScope instance;
};

}

namespace gui {

static auto& Scope = internal::InternalScope::instance;

}