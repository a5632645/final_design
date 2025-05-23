#pragma once
#include <cstdint>
#include <span>
#include <string_view>
#include "dsp/ParamDesc.hpp"
#include "bsp/oled/OLEDDisplay.h"

namespace gui {

class TimeKnob {
public:
    void BindParamDesc(dsp::FloatParamDesc& desc) {
        obj_ = &desc;
    }
    void BindParamDesc(dsp::IntParamDesc& desc) {
        obj_ = &desc;
    }
    void BindParamDesc(dsp::BoolParamDesc& desc) {
        obj_ = &desc;
    }
    dsp::IParamDesc* GetParamDesc() const {
        return obj_;
    }

    void Add(int32_t value, bool alt) {
        if (obj_ == nullptr) return;
        obj_->Add(value, alt);
        Begin();
    }
    void Reset() {
        if (obj_ == nullptr) return;
        obj_->Reset();
        Begin();
    }

    void Begin(uint32_t ms = 1000) {
        msStill_ = ms;
    }

    void SetExpired() {
        msStill_ = -1;
    }

    bool IsExpired() {
        return msStill_ < 0;
    }

    bool Tick(uint32_t msEscape) {
        if (msStill_ < 0) return false;
        msStill_ -= msEscape;
        if (msStill_ < 0) return true;
        return false;
    }

    void Draw(OLEDDisplay& display, Rectange aera);
    void Draw(OLEDDisplay& display, dsp::FloatParamDesc& desc, Rectange aera);
    void Draw(OLEDDisplay& display, dsp::IntParamDesc& desc, Rectange aera);
    void DrawWTF(OLEDDisplay& display, dsp::IntParamDesc& desc, Rectange aera, std::span<const std::string_view> str);
    void Draw(OLEDDisplay& display, dsp::BoolParamDesc& desc, Rectange aera);
private:
    int32_t msStill_ = -1;
    dsp::IParamDesc* obj_ = nullptr;
};

} // namespace gui
