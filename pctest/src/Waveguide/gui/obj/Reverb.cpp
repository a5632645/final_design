#include "Reverb.hpp"
#include "bsp/ControlIO.hpp"
#include "dsp/params.hpp"
#include "utli/Lerp.hpp"
#include "dsp/Body.hpp"

enum PageIdx {
    ePage_Reverb = 0,
    ePage_Convolution,
    ePage_NumPages
};

static constexpr float FastSin(float omega) {
    return std::sin(omega);
}

static constexpr float FastCos(float omega) {
    return std::cos(omega);
}

static constexpr float kEndAngle = -std::numbers::pi_v<float> / 4.0f;
static constexpr float kBeginAngle = 5.0f * std::numbers::pi_v<float> / 4.0f;

namespace gui::internal {

void CReverb::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    drawer.DrawBottomOptions(0, "混响");
    drawer.DrawBottomOptions(1, "琴体");

    if (pageIdx_ == ePage_Convolution) {
        drawer.DrawTitleBar("琴体");
        auto rect = drawer.display.GetDrawAera();
        rect.RemovedFromTop(12);
        rect.RemovedFromBottom(12);
        rect.h = rect.h / 3;
        rect.w = rect.w / 4;
        for (int8_t i = 0; i < 4; ++i) {
            if (i == selectIdx_) {
                drawer.display.setColor(OledColorEnum::kOledWHITE);
                drawer.display.drawRect(rect.x, rect.y, rect.w, rect.h);
            }
            if (i == 1) {
                auto r = rect;
                auto& display = drawer.display;
                auto& desc = dsp::SynthParams.bodyType;
                display.setColor(OledColorEnum::kOledWHITE);
                auto oldAlign = display.GetTextAlignment();
                display.setTextAlignment(OledDisplayTextAlignEnum::kXCenter);

                auto textAera = r.RemoveFromTop(display.GetFontHeight());
                if (knobs_[1].IsExpired()) { // draw text
                    display.drawStringMaxWidth(textAera.x, textAera.y, textAera.w, desc.name);
                }
                else { // draw number
                    auto span = display.FormantToInternalBuffer("{}", dsp::Body::GetName(desc.Get()));
                    display.drawStringMaxWidth(textAera.x, textAera.y, textAera.w, std::string_view(span.data(), span.size()));
                }
                auto center = r.GetCenter();
                auto radius = r.h / 2;
                display.drawCircle(center.x, center.y, radius);
                auto norm = desc.GetValueAsNormalized();
                auto angle = utli::Lerp(kBeginAngle, kEndAngle, norm);
                auto x = FastCos(angle) * radius + center.x;
                auto y = -FastSin(angle) * radius + center.y;
                display.drawLine(center.x, center.y, x, y);
                display.setTextAlignment(oldAlign);
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
    else if (pageIdx_ == ePage_Reverb) {
        drawer.DrawTitleBar("混响");
        auto rect = drawer.display.GetDrawAera();
        rect.RemovedFromTop(12);
        rect.RemovedFromBottom(12);
        rect.h = rect.h / 3;
        rect.w = rect.w / 4;
        for (int8_t i = 0; auto& k : knobs_) {
            if (i == selectIdx_) {
                drawer.display.setColor(OledColorEnum::kOledWHITE);
                drawer.display.drawRect(rect.x, rect.y, rect.w, rect.h);
            }
            k.Draw(drawer.display, rect);
            rect.Translated(rect.w, 0);
            ++i;
            if (i % 4 == 0) {
                rect.x = 0;
                rect.Translated(0, rect.h);
            }
        }
    }
}

void CReverb::OnSelect() {
    using bsp::ControlIO, dsp::SynthParams;
    SetPageIdx(pageIdx_);

    ControlIO.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue) {
        Reverb.selectIdx_ = std::clamp<int32_t>(Reverb.selectIdx_ + dvalue, 0, std::size(Reverb.knobs_) - 1);
        Reverb.Redraw();
    });
    ControlIO.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue) {
        Reverb.knobs_[Reverb.selectIdx_].Add(dvalue, false);
        Reverb.knobs_[Reverb.selectIdx_].Begin();
        Reverb.Redraw();
    });
    ControlIO.SetButtonCallback(bsp::ButtonId::kBtn0, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            Reverb.SetPageIdx(ePage_Reverb);
        }
    });
    ControlIO.SetButtonCallback(bsp::ButtonId::kBtn1, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            Reverb.SetPageIdx(ePage_Convolution);
        }
    });
}

void CReverb::OnTimeTick(uint32_t msEscape) {
    for (auto& k : knobs_) {
        if (k.Tick(msEscape)) {
            Reverb.Redraw();
        }
    }
}

void CReverb::SetPageIdx(int8_t idx) {
    pageIdx_ = std::clamp<int8_t>(idx, 0, ePage_NumPages - 1);
    Reverb.Redraw();

    using bsp::ControlIO, dsp::SynthParams;
    if (pageIdx_ == ePage_Convolution) {
        knobs_[0].BindParamDesc(SynthParams.body);
        knobs_[1].BindParamDesc(SynthParams.bodyType);
        knobs_[2].BindParamDesc(SynthParams.wetGain);
        knobs_[3].BindParamDesc(SynthParams.stretch);
        selectIdx_ = std::clamp<int8_t>(selectIdx_, 0, 3);
    }
    else if (pageIdx_ == ePage_Reverb) {
        knobs_[0].BindParamDesc(SynthParams.reverb.decay);
        knobs_[1].BindParamDesc(SynthParams.reverb.interval);
        knobs_[2].BindParamDesc(SynthParams.reverb.lossLP);
        knobs_[3].BindParamDesc(SynthParams.reverb.rate);
        knobs_[4].BindParamDesc(SynthParams.reverb.drywet);
        knobs_[5].BindParamDesc(SynthParams.reverb.earlyRefl);
        knobs_[6].BindParamDesc(SynthParams.reverb.depth);
        knobs_[7].BindParamDesc(SynthParams.reverb.size);
        knobs_[8].BindParamDesc(SynthParams.reverb.chrousDept);
        knobs_[9].BindParamDesc(SynthParams.reverb.chrousRate);
        selectIdx_ = std::clamp<int8_t>(selectIdx_, 0, 9);
    }
}

}