#include "Reed.hpp"
#include "Styles.hpp"
#include "dsp/params.hpp"
#include "ReedDebug.hpp"

enum PageIdx {
    ePage_page1 = 0,
    ePage_page2,
    ePage_NumPages
};

namespace gui::internal {

void CReed::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    auto& params = dsp::SynthParams;
    drawer.DrawTitleBar("木管");

    drawer.DrawBottomOptions(0, "1");
    drawer.DrawBottomOptions(1, "2");
    drawer.DrawBottomOptions(2, "反射表");
    
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

void CReed::OnSelect() {
    SetPageIdx(pageIdx_);

    auto& io = bsp::ControlIO;
    io.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue){
        Reed.selectIdx_ += dvalue;
        if (Reed.selectIdx_ < 0) Reed.selectIdx_ = 0;
        if (Reed.selectIdx_ >= Reed.totalKnobs_) Reed.selectIdx_ = Reed.totalKnobs_ - 1;
        Reed.Redraw();
    });
    io.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue){
        Reed.knobs_[Reed.selectIdx_].Add(dvalue, false);
        Reed.Redraw();
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn0, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            Reed.SetPageIdx(ePage_page1);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn1, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            Reed.SetPageIdx(ePage_page2);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn2, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(ReedDebug);
        } 
    });
}

void CReed::OnTimeTick(uint32_t msEscape) {
    for (auto& k : knobs_) {
        if (k.Tick(msEscape)) {
            Reed.Redraw();
        }
    }
}

void CReed::SetPageIdx(int8_t page) {
    auto& params = dsp::SynthParams;

    if (pageIdx_ != page) {
        pageIdx_ = page;
        Reed.Redraw();

        switch (page) {
        case ePage_page1:
            knobs_[0].BindParamDesc(params.reed.lossHP);
            knobs_[1].BindParamDesc(params.reed.lossGain);
            knobs_[2].BindParamDesc(params.reed.lossLP);
            knobs_[3].BindParamDesc(params.reed.noiseGain);
            knobs_[4].BindParamDesc(params.reed.airGain);
            knobs_[6].BindParamDesc(params.reed.attack);
            knobs_[7].BindParamDesc(params.reed.release);
            knobs_[8].BindParamDesc(params.reed.inhalling);
            knobs_[9].BindParamDesc(params.reed.active);
            knobs_[10].BindParamDesc(params.reed.blend);
            knobs_[11].BindParamDesc(params.reed.lossFaster);
            totalKnobs_ = 12;
            break;
        case ePage_page2:
            knobs_[0].BindParamDesc(params.reed.manualPres);
            knobs_[1].BindParamDesc(params.reed.autoVibrate);
            knobs_[2].BindParamDesc(params.reed.vibrateRate);
            knobs_[3].BindParamDesc(params.reed.vibrateDepth);
            knobs_[4].BindParamDesc(params.reed.autoTremolo);
            knobs_[5].BindParamDesc(params.reed.tremoloRate);
            knobs_[6].BindParamDesc(params.reed.tremoloDepth);
            knobs_[7].BindParamDesc(params.reed.vibrateAttack);
            knobs_[8].BindParamDesc(params.reed.tremoloAttack);
            totalKnobs_ = 9;
            break;
        }

        selectIdx_ = std::clamp(selectIdx_, 0, totalKnobs_ - 1);
    }
}

}