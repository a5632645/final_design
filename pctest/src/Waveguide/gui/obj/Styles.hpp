#pragma once
#include "oled/OLEDDisplay.h"

namespace gui {

struct StyleDrawer {
    void DrawOnOffButton(Rectange aera, bool on, std::string_view onText, std::string_view offText);
    void DrawButton(Rectange aera, bool on, std::string_view text);
    void DrawSlider(Rectange aera, int16_t width, std::string_view text);
    template<class... Args> USF_CPP14_CONSTEXPR
    void DrawFormatSlider(Rectange aera, int16_t width, usf::StringView text, Args&&... args) {
        auto span = display.FormantToInternalBuffer(text, std::forward<Args>(args)...);
        DrawSlider(aera, width, std::string_view(span.data(), span.size()));
    }    
    void DrawTitleBar(std::string_view text);
    void DrawBottomOptions(uint32_t idx, std::string_view text);

    void Inverse(Rectange rect);
    void InverseFrame(Rectange rect);

    explicit StyleDrawer(OLEDDisplay& display) : display(display) {}
    OLEDDisplay& display;
};

}
