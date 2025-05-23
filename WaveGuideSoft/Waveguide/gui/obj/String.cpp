#include "String.hpp"
#include "Styles.hpp"
#include "dsp/params.hpp"

enum PageIdx {
    ePage_page1 = 0,
    ePage_page2,
    ePage_NumPages
};

namespace gui::internal {

void CString::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    drawer.DrawTitleBar("string");

    drawer.DrawBottomOptions(0, "1");
    drawer.DrawBottomOptions(1, "2");
    
    auto rect = drawer.display.GetDrawAera();
    rect.RemovedFromTop(12);
    rect.RemovedFromBottom(12);
    rect.h = rect.h / 3;
    rect.w = rect.w / 4;

    for (int8_t i = 0; i < totalKnobs_; ++i) {
        if (i == selectIdx_) {
            drawer.display.setColor(OledColorEnum::kOledWHITE);
            drawer.display.drawRect(rect.x, rect.y, rect.w, rect.h);
        }
        knobs_[i].Draw(drawer.display, rect);
        rect.Translated(rect.w, 0);
        if (i % 4 == 3) {
            rect.x = 0;
            rect.Translated(0, rect.h);
        }
    }
}

void CString::OnSelect() {
    SetPageIdx(pageIdx_);

    auto& io = bsp::ControlIO;
    io.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue){
        String.selectIdx_ += dvalue;
        if (String.selectIdx_ < 0) String.selectIdx_ = 0;
        if (String.selectIdx_ >= String.totalKnobs_) String.selectIdx_ = String.totalKnobs_ - 1;
        String.Redraw();
    });
    io.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue){
        String.knobs_[String.selectIdx_].Add(dvalue, false);
        String.Redraw();
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn0, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            String.SetPageIdx(ePage_page1);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn1, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            String.SetPageIdx(ePage_page2);
        }
    });
}

void CString::OnTimeTick(uint32_t msEscape) {
    for (auto& k : knobs_) {
        if (k.Tick(msEscape)) {
            String.Redraw();
        }
    }
}

void CString::SetPageIdx(int8_t page) {
    auto& params = dsp::SynthParams;

    pageIdx_ = page;
    String.Redraw();

    switch (page) {
    case ePage_page1:
        knobs_[0].BindParamDesc(params.string.decay);
        knobs_[1].BindParamDesc(params.string.exciFaster);
        knobs_[2].BindParamDesc(params.string.dispersion);
        knobs_[3].BindParamDesc(params.string.pos);
        knobs_[4].BindParamDesc(params.string.color);
        knobs_[5].BindParamDesc(params.string.lossFaster);
        knobs_[6].BindParamDesc(params.string.exciTructionlow);
        knobs_[7].BindParamDesc(params.string.exciTructionHigh);
        knobs_[8].BindParamDesc(params.string.exciOutLow);
        knobs_[9].BindParamDesc(params.string.exciOutHigh);
        knobs_[10].BindParamDesc(params.string.vibrateDepth);
        totalKnobs_ = 11;
        break;
    case ePage_page2:
        knobs_[0].BindParamDesc(params.string.lossTructionlow);
        knobs_[1].BindParamDesc(params.string.lossTructionHigh);
        knobs_[2].BindParamDesc(params.string.lossOutLow);
        knobs_[3].BindParamDesc(params.string.lossOutHigh);
        totalKnobs_ = 4;
        break;

        selectIdx_ = std::clamp<int8_t>(selectIdx_, 0, totalKnobs_ - 1);
    }
}

}