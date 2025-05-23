#include "Scope.hpp"
#include "utli/Clamp.hpp"
#include "bsp/ControlIO.hpp"

enum ScopeSelectOptions {
    eScopeSelect_SampleInterval = 0,
    eScopeSelect_Scale,
    eScopeSelect_NumOptions
};

namespace gui::internal {

void CScope::Draw(StyleDrawer& display) {
    display.display.Fill(OledColorEnum::kOledBLACK);
    auto rect = display.display.GetDrawAera();
    {
        auto t = rect.RemoveFromTop(12);
        display.display.setColor(OledColorEnum::kOledWHITE);
        t = t.RemoveFromLeft(t.w / 2);
        display.display.FormatString(t.x, t.y, "间隔{}采样", sampleInterval_);
        if (selectIdx_ == eScopeSelect_SampleInterval) display.InverseFrame(t);
        t.Translated(t.w, 0);
        display.display.FormatString(t.x, t.y, "缩放{}dB", dBSclae_);
        if (selectIdx_ == eScopeSelect_Scale) display.InverseFrame(t);
    }
    float maxSample = 0.0f;
    { // waveform
        auto scopeRect = rect.RemoveFromLeft(scopeBuffer_.size() + 2);
        display.display.setColor(OledColorEnum::kOledWHITE);
        display.display.drawRect(scopeRect.x, scopeRect.y, scopeRect.w, scopeRect.h);
        auto center = scopeRect.GetCenter();
        for (uint32_t i = 0; i < scopeBuffer_.size(); ++i) {
            auto h = static_cast<int16_t>(scopeBuffer_[i] * center.y);
            auto x = scopeRect.x + i + 1;
            if (h > 0) {
                display.display.drawVerticalLine(x, center.y - h, h);
            }
            else {
                if (h == 0) h = -1;
                display.display.drawVerticalLine(x, center.y, -h);
            }
            maxSample = std::max(maxSample, std::abs(scopeBuffer_[i]));
        }
        writePos_ = 0;
    }
    {
        auto peakRect = rect.RemoveFromLeft(8);
        display.display.setColor(OledColorEnum::kOledWHITE);
        display.display.drawRect(peakRect.x, peakRect.y, peakRect.w, peakRect.h);
        auto bottomY = peakRect.y + peakRect.h;
        auto topY = bottomY - peakRect.h * maxSample / 2.0f;
        display.display.fillRect(peakRect.x, topY, peakRect.w, bottomY - topY);
    }
}

void CScope::OnSelect() {
    using bsp::ControlIO;
    ControlIO.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue) {
        switch (Scope.selectIdx_) {
        case eScopeSelect_SampleInterval:
            Scope.sampleInterval_ = utli::Clamp(Scope.sampleInterval_ + dvalue, 1, 200);
            break;
        case eScopeSelect_Scale:
            Scope.dBSclae_ = utli::Clamp(Scope.dBSclae_ + dvalue, 0, 24);
            Scope.sclae_ = std::pow(10.0f, Scope.dBSclae_ / 20.0f);
            break;
        }
        Scope.Redraw();
    });
    ControlIO.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue) {
        Scope.selectIdx_ = utli::Clamp(Scope.selectIdx_ + dvalue, 0, ScopeSelectOptions::eScopeSelect_NumOptions - 1);
        Scope.Redraw();
    });
}

void CScope::Push(std::span<float> block) {
    float last = 0.0f;
    for (uint32_t i = 0; i < block.size(); ++i) {
        if (last * block[i] < 0.0f) {
            for (;i < block.size(); i += sampleInterval_) {
                if (writePos_ == scopeBuffer_.size()) {
                    Redraw();
                    return;
                }
                scopeBuffer_[writePos_++] = block[i] * sclae_;
            }
            return;
        }
        last = block[i];
    }
}
}