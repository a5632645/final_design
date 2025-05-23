#include "BowedDebug.hpp"
#include "bsp/ControlIO.hpp"
#include "dsp/params.hpp"
#include "dsp/Synth.hpp"
#include "Bowed.hpp"

namespace gui::internal {
    
void CBowedDebug::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    drawer.DrawBottomOptions(0, "返回");
    
    auto rect = drawer.display.GetDrawAera();
    rect.RemovedFromBottom(12);
    {
        auto knobRect = rect.RemoveFromTop(30);
        auto e = knobRect.RemoveFromLeft(30);
        for (int32_t idx = 0; auto& k : knobs_) {
            k.Draw(drawer.display, e);
            if (selectIdx_ == idx) drawer.InverseFrame(e);
            ++idx;
            e = knobRect.RemoveFromLeft(30);
        }
    }

    {
        rect.Reduced(2, 2);
        drawer.display.setColor(OledColorEnum::kOledWHITE);
        drawer.display.drawHorizontalLine(rect.x, rect.y + rect.h, rect.w);
        drawer.display.drawVerticalLine(rect.GetCenter().x, rect.y, rect.h);

        for (int32_t i = 0; i < rect.w; ++i) {
            auto x = rect.x + i;
            auto delta = std::lerp(-1, 1, static_cast<float>(i) / rect.w);
            auto reflections = dsp::Synth.GetBowed().BowReflectionTable(delta);
            auto y = std::lerp(rect.y + rect.h, rect.y, reflections);
            drawer.display.setPixel(x, y);
        }

        auto delta = dsp::Synth.GetBowed().deltaDebugValue_;
        delta = delta * 0.5f + 0.5f;
        auto x = std::lerp(rect.x, rect.x + rect.w, delta);
        drawer.display.drawVerticalLine(x, rect.y, rect.h);
    }
    Redraw();
}

void CBowedDebug::OnSelect() {
    using bsp::ControlIO;
    ControlIO.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue) {
        BowedDebug.selectIdx_ = std::clamp<int32_t>(BowedDebug.selectIdx_ + dvalue, 0, std::size(BowedDebug.knobs_) - 1);
        BowedDebug.Redraw();
    });
    ControlIO.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue) {
        BowedDebug.knobs_[BowedDebug.selectIdx_].Add(dvalue, false);
        BowedDebug.knobs_[BowedDebug.selectIdx_].Begin();
        BowedDebug.Redraw();
    });
    ControlIO.SetButtonCallback(bsp::ButtonId::kBtn0, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(Bowed);
        }
    });

    knobs_[0].BindParamDesc(dsp::SynthParams.bow.bowSpeed);
    knobs_[1].BindParamDesc(dsp::SynthParams.bow.bowPos);
    knobs_[2].BindParamDesc(dsp::SynthParams.bow.offset);
    knobs_[3].BindParamDesc(dsp::SynthParams.bow.slope);
    knobs_[4].BindParamDesc(dsp::SynthParams.bow.reflectMin);
    knobs_[5].BindParamDesc(dsp::SynthParams.bow.reflectMax);
    knobs_[6].BindParamDesc(dsp::SynthParams.body);
}

void CBowedDebug::OnTimeTick(uint32_t msEscape) {
    for (auto& k : knobs_) {
        if (k.Tick(msEscape)) {
            BowedDebug.Redraw();
        }
    }
}

}