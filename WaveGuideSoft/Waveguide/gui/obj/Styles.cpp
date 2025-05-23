#include "Styles.hpp"

using enum OledColorEnum;

namespace gui {

void StyleDrawer::DrawOnOffButton(Rectange aera, bool on, std::string_view onText, std::string_view offText) {
    if (on) {
        display.setColor(kOledWHITE);
        display.fillRect(aera.x, aera.y, aera.w, aera.h);
        display.setColor(kOledBLACK);
        aera.Reduced(2,0);
        display.drawString(aera.x, aera.y, onText);
    }
    else {
        display.setColor(kOledBLACK);
        display.fillRect(aera.x, aera.y, aera.w, aera.h);
        display.setColor(kOledWHITE);
        display.drawRect(aera.x, aera.y, aera.w, aera.h);
        display.setColor(kOledWHITE);
        aera.Reduced(2,0);
        display.drawString(aera.x, aera.y, offText);
    }
}

void StyleDrawer::DrawButton(Rectange aera, bool on, std::string_view text) {
    DrawOnOffButton(aera, on, text, text);
}

void StyleDrawer::DrawSlider(Rectange aera, int16_t width, std::string_view text) {
    auto box = aera;
    box.Reduced(1, 1);
    display.setColor(kOledWHITE);
    display.drawString(box.x, box.y, text);

    display.setColor(kOledINVERSE);
    display.fillRect(aera.x, aera.y, width, aera.h);

    display.setColor(kOledWHITE);
    display.drawRect(aera.x, aera.y, aera.w, aera.h);
}


void StyleDrawer::DrawTitleBar(std::string_view text) {
    auto aera = display.GetDrawAera().WithHeight(12);
    display.setColor(kOledWHITE);
    display.fillRect(aera.x, aera.y, aera.w, aera.h);
    display.setColor(kOledBLACK);
    display.drawString(aera.x, aera.y, text);
}

void StyleDrawer::DrawBottomOptions(uint32_t idx, std::string_view text) {
    display.setColor(kOledWHITE);
    auto x = idx * display.getWidth() / 5;
    display.drawString(x, display.getHeight() - 12, text);
}

Rectange StyleDrawer::GetBottomRect(uint32_t idx) {
    display.setColor(kOledWHITE);
    auto w = display.getWidth() / 5;
    auto x = idx * w;
    auto y = display.getHeight() - 12;
    return Rectange(x, y, w, 12);
}

void StyleDrawer::Inverse(Rectange rect) {
    auto c = display.getColor();
    display.setColor(kOledINVERSE);
    display.fillRect(rect.x, rect.y, rect.w, rect.h);
    display.setColor(c);
}

void StyleDrawer::InverseFrame(Rectange rect) {
    auto c = display.getColor();
    display.setColor(kOledINVERSE);
    display.drawRect(rect.x, rect.y, rect.w, rect.h);
    display.setColor(c);
}

}
