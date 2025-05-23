#include "Bowed.hpp"
#include "Styles.hpp"
#include "gui/GuiDispatch.hpp"
#include "dsp/params.hpp"
#include "BowedDebug.hpp"

enum PageIdx {
    ePage_page1 = 0,
    ePage_page2,
    ePage_page3,
    ePage_NumPages
};

namespace gui::internal {

void CBowed::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    drawer.DrawTitleBar("bow");

    drawer.DrawBottomOptions(0, "1");
    drawer.DrawBottomOptions(1, "2");
    drawer.DrawBottomOptions(2, "3");
    drawer.DrawBottomOptions(3, "反射表");
    
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
        if (knobs_[i].GetParamDesc() == &dsp::SynthParams.bow.vibrateControl) {
            knobs_[i].DrawWTF(drawer.display, dsp::SynthParams.bow.vibrateControl, rect, dsp::kVibrateModeStrs);
        }
        else if (knobs_[i].GetParamDesc() == &dsp::SynthParams.bow.tremoloControl) {
            knobs_[i].DrawWTF(drawer.display, dsp::SynthParams.bow.tremoloControl, rect, dsp::kTremoloModeStrs);
        }
        else {
            knobs_[i].Draw(drawer.display, rect);
        }
        rect.Translated(rect.w, 0);
        if (i % 4 == 3) {
            rect.x = 0;
            rect.Translated(0, rect.h);
        }
    }
}

void CBowed::OnSelect() {
    SetPageIdx(pageIdx_);

    auto& io = bsp::ControlIO;
    io.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue){
        Bowed.selectIdx_ += dvalue;
        if (Bowed.selectIdx_ < 0) Bowed.selectIdx_ = 0;
        if (Bowed.selectIdx_ >= Bowed.totalKnobs_) Bowed.selectIdx_ = Bowed.totalKnobs_ - 1;
        Bowed.Redraw();
    });
    io.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue){
        Bowed.knobs_[Bowed.selectIdx_].Add(dvalue, false);
        Bowed.Redraw();
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn0, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            Bowed.SetPageIdx(ePage_page1);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn1, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            Bowed.SetPageIdx(ePage_page2);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn2, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            Bowed.SetPageIdx(ePage_page3);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn3, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(BowedDebug);
        } 
    });
}

void CBowed::OnTimeTick(uint32_t msEscape) {
    for (auto& k : knobs_) {
        if (k.Tick(msEscape)) {
            Redraw();
        }
    }
}

void CBowed::SetPageIdx(int8_t page) {
    auto& params = dsp::SynthParams;

    pageIdx_ = page;
    Redraw();

    switch (page) {
    case ePage_page1:
        knobs_[0].BindParamDesc(params.bow.bowPos);
        knobs_[1].BindParamDesc(params.bow.bowSpeed);
        knobs_[2].BindParamDesc(params.bow.offset);
        knobs_[3].BindParamDesc(params.bow.slope);
        knobs_[4].BindParamDesc(params.bow.reflectMin);
        knobs_[5].BindParamDesc(params.bow.reflectMax);
        knobs_[6].BindParamDesc(params.bow.decay);
        knobs_[7].BindParamDesc(params.bow.lossFaster);
        knobs_[8].BindParamDesc(params.bow.noise);
        knobs_[9].BindParamDesc(params.bow.noiseLP);
        totalKnobs_ = 10;
        break;
    case ePage_page2:
        knobs_[0].BindParamDesc(params.bow.vibrateControl);
        knobs_[1].BindParamDesc(params.bow.vibrateRate);
        knobs_[2].BindParamDesc(params.bow.vibrateDepth);
        knobs_[3].BindParamDesc(params.bow.vibrateAttack);
        knobs_[4].BindParamDesc(params.bow.tremoloControl);
        knobs_[5].BindParamDesc(params.bow.tremoloRate);
        knobs_[6].BindParamDesc(params.bow.tremoloDepth);
        knobs_[7].BindParamDesc(params.bow.tremoloAttack);
        knobs_[8].BindParamDesc(params.bow.attack);
        knobs_[9].BindParamDesc(params.bow.release);
        totalKnobs_ = 10;
        break;
    case ePage_page3:
        knobs_[0].BindParamDesc(params.bow.lossTructionlow);
        knobs_[1].BindParamDesc(params.bow.lossTructionHigh);
        knobs_[2].BindParamDesc(params.bow.lossOutLow);
        knobs_[3].BindParamDesc(params.bow.lossOutHigh);
        totalKnobs_ = 4;
        break;
    }

    selectIdx_ = std::clamp<int8_t>(selectIdx_, 0, totalKnobs_ - 1);
}

}