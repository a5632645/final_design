#include "ReedDebug.hpp"
#include "dsp/params.hpp"
#include "dsp/Synth.hpp"
#include "bsp/ControlIO.hpp"
#include "Reed.hpp"

namespace gui::internal {

void CReedDebug::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    drawer.DrawBottomOptions(0, "返回");

    auto rect = drawer.display.GetDrawAera();
    rect.RemovedFromBottom(12);
    {
        auto knobRect = rect.RemoveFromTop(30);
        auto e = knobRect.RemoveFromLeft(30);
        for (int idx = 0; auto& k : knobs_) {
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

        for (int i = 0; i < rect.w; ++i) {
            auto x = rect.x + i;
            auto delta = std::lerp(-1, 1, static_cast<float>(i) / rect.w);
            auto reflectionCoeff = dsp::Synth.GetReed().ReedReflection2(delta);
            auto y = std::lerp(rect.y + rect.h, rect.y, reflectionCoeff);
            drawer.display.setPixel(x, y);
        }
        
        auto debugValue = dsp::Synth.GetReed().debugValue_;
        auto x = std::lerp(rect.x, rect.x + rect.w, debugValue * 0.5f + 0.5f);
        drawer.display.drawVerticalLine(x, rect.y, rect.h);
        auto outputWave = dsp::Synth.GetReed().debugValueOutputWave_;
        outputWave = outputWave > 0.0f ? 0.8f : 0.2f;
        auto y = std::lerp(rect.y + rect.h, rect.y, outputWave);
        drawer.display.drawHorizontalLine(rect.x, y, rect.w);
    }

    ReedDebug.Redraw();
}

void CReedDebug::OnSelect() {
    knobs_[0].BindParamDesc(dsp::SynthParams.reed.inhalling);
    knobs_[1].BindParamDesc(dsp::SynthParams.reed.active);
    knobs_[2].BindParamDesc(dsp::SynthParams.reed.blend);
    knobs_[3].BindParamDesc(dsp::SynthParams.reed.airGain);

    using bsp::ControlIO;
    ControlIO.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue) {
        ReedDebug.selectIdx_ = std::clamp<int32_t>(ReedDebug.selectIdx_ + dvalue, 0, std::size(ReedDebug.knobs_) - 1);
        ReedDebug.Redraw();
    });
    ControlIO.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue) {
        ReedDebug.knobs_[ReedDebug.selectIdx_].Add(dvalue, false);
        ReedDebug.knobs_[ReedDebug.selectIdx_].Begin();
        ReedDebug.Redraw();
    });
    ControlIO.SetButtonCallback(bsp::ButtonId::kBtn0, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(Reed);
        }
    });
}

void CReedDebug::OnTimeTick(uint32_t msEscape) {
    for (auto& k : knobs_) {
        if (k.Tick(msEscape)) {
            ReedDebug.Redraw();
        }
    }
}

}