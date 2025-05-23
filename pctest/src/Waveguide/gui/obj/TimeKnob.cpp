#include "TimeKnob.hpp"
#include <numbers>
#include "dsp/Util.hpp"

static constexpr float FastSin(float omega) {
    return std::sin(omega);
}

static constexpr float FastCos(float omega) {
    return std::cos(omega);
}

static constexpr float kEndAngle = -std::numbers::pi_v<float> / 4.0f;
static constexpr float kBeginAngle = 5.0f * std::numbers::pi_v<float> / 4.0f;

void gui::TimeKnob::Draw(OLEDDisplay& display, Rectange aera) {
    if (obj_ == nullptr) {
        display.setColor(OledColorEnum::kOledWHITE);
        display.drawRect(aera.x, aera.y, aera.w, aera.h);
        display.drawStringMaxWidth(aera.x, aera.y, aera.w, "null param");
        return;
    }

    if (obj_->GetParamType() == dsp::ParamType::kFloat) {
        Draw(display, obj_->As<dsp::FloatParamDesc>(), aera);
    }
    else if (obj_->GetParamType() == dsp::ParamType::kInt) {
        Draw(display, obj_->As<dsp::IntParamDesc>(), aera);
    }
    else if (obj_->GetParamType() == dsp::ParamType::kBool) {
        Draw(display, obj_->As<dsp::BoolParamDesc>(), aera);
    }
}

void gui::TimeKnob::Draw(OLEDDisplay &display, dsp::FloatParamDesc &desc, Rectange aera)
{
    display.setColor(OledColorEnum::kOledWHITE);
    auto oldAlign = display.GetTextAlignment();
    display.setTextAlignment(OledDisplayTextAlignEnum::kXCenter);

    auto textAera = aera.RemoveFromTop(display.GetFontHeight());
    if (IsExpired()) { // draw text
        display.drawStringMaxWidth(textAera.x, textAera.y, textAera.w, desc.name);
    }
    else { // draw number
        auto span = display.FormantToInternalBuffer("{}", desc.Get());
        display.drawStringMaxWidth(textAera.x, textAera.y, textAera.w, std::string_view(span.data(), span.size()));
    }
    auto center = aera.GetCenter();
    auto radius = aera.h / 2;
    display.drawCircle(center.x, center.y, radius);
    auto norm = desc.GetValueAsNormalized();
    auto angle = dsp::utli::LerpUncheck(kBeginAngle, kEndAngle, norm);
    auto x = FastCos(angle) * radius + center.x;
    auto y = -FastSin(angle) * radius + center.y;
    display.drawLine(center.x, center.y, x, y);
    display.setTextAlignment(oldAlign);
}

void gui::TimeKnob::Draw(OLEDDisplay& display, dsp::IntParamDesc& desc, Rectange aera) {
    display.setColor(OledColorEnum::kOledWHITE);
    auto oldAlign = display.GetTextAlignment();
    display.setTextAlignment(OledDisplayTextAlignEnum::kXCenter);

    auto textAera = aera.RemoveFromTop(display.GetFontHeight());
    if (IsExpired()) { // draw text
        display.drawStringMaxWidth(textAera.x, textAera.y, textAera.w, desc.name);
    }
    else { // draw number
        auto span = display.FormantToInternalBuffer("{}", desc.Get());
        display.drawStringMaxWidth(textAera.x, textAera.y, textAera.w, std::string_view(span.data(), span.size()));
    }
    auto center = aera.GetCenter();
    auto radius = aera.h / 2;
    display.drawCircle(center.x, center.y, radius);
    auto norm = desc.GetValueAsNormalized();
    auto angle = dsp::utli::LerpUncheck(kBeginAngle, kEndAngle, norm);
    auto x = FastCos(angle) * radius + center.x;
    auto y = -FastSin(angle) * radius + center.y;
    display.drawLine(center.x, center.y, x, y);
    display.setTextAlignment(oldAlign);
}

void gui::TimeKnob::Draw(OLEDDisplay& display, dsp::BoolParamDesc& desc, Rectange aera) {
    display.setColor(OledColorEnum::kOledWHITE);
    auto oldAlign = display.GetTextAlignment();
    display.setTextAlignment(OledDisplayTextAlignEnum::kXCenter);

    auto textAera = aera.RemoveFromTop(display.GetFontHeight());
    if (IsExpired()) { // draw text
        display.drawStringMaxWidth(textAera.x, textAera.y, textAera.w, desc.name);
    }
    else { // draw number
        auto span = display.FormantToInternalBuffer("{}", desc.Get());
        display.drawStringMaxWidth(textAera.x, textAera.y, textAera.w, std::string_view(span.data(), span.size()));
    }
    auto center = aera.GetCenter();
    auto radius = aera.h / 2;
    display.drawCircle(center.x, center.y, radius);
    auto norm = desc.Get() ? 1.0f : 0.0f;
    auto angle = dsp::utli::LerpUncheck(kBeginAngle, kEndAngle, norm);
    auto x = FastCos(angle) * radius + center.x;
    auto y = -FastSin(angle) * radius + center.y;
    display.drawLine(center.x, center.y, x, y);
    display.setTextAlignment(oldAlign);
}
