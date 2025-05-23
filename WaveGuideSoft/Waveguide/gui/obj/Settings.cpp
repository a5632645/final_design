#include "Settings.hpp"
#include "bsp/Keyboard.hpp"
#include "bsp/ControlIO.hpp"
#include "bsp/MPR121.hpp"
#include "utli/Clamp.hpp"
#include "MidiManager.hpp"
#include "bsp/UC1638.hpp"
#include "dsp/params.hpp"

enum AdcSelectOptions {
    eAdcSelect_ADCUpValue = 0,
    eAdcSelect_ADCDownValue,
    eAdcSelect_MinADCValue,
    eAdcSelect_NumOptions
};

enum PadSelectOptions {
    ePadSelect_TouchPadFilter = 0,
    ePadSelect_ModWheelFilter,
    ePadSelect_PitchBendFilter,
    ePadSelect_NumOptions
};

enum SliderSelectOptions {
    eSliderSelect_TouchSliderFilter = 0,
    eSliderSelect_NumOptions
};

enum ScreenSelectOptions {
    eScreen_MirrorX = 0,
    eScreen_MirrorY,
    eScreen_BackLight,
    eScreen_NumOptions
};

enum MPESelectOptions {
    eMPE_MPE = 0,
    eMPE_Play,
    eMPE_PitchBendRange,
    eMPE_NumOptions
};

enum Page {
    ePage_Adc = 0,
    ePage_TouchPad,
    ePage_TouchSlider,
    ePage_Mpe,
    ePage_Screen
};

static constexpr int32_t kWhiteKeyIdxTable[7] = {0, 2, 4, 5, 7, 9, 11};
static constexpr int32_t kBlackKeyIdxTable[5] = {1, 3, 6, 8, 10};

using bsp::ControlIO;
using bsp::Keyboard;

namespace gui {

void CSettings::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    drawer.DrawTitleBar("keyboard settings");

    auto rect = drawer.display.GetDrawAera();
    rect.RemovedFromTop(12);
    rect.RemovedFromBottom(12);
    {
        drawer.display.setColor(OledColorEnum::kOledWHITE);
        drawer.DrawBottomOptions(0, "adc");
        drawer.DrawBottomOptions(1, "pad");
        drawer.DrawBottomOptions(2, "滑条");
        drawer.DrawBottomOptions(3, "mpe");
        drawer.DrawBottomOptions(4, "屏幕");
        auto mpeRect = drawer.GetBottomRect(3);
        if (Keyboard.IsMPEEnabled()) {
            drawer.Inverse(mpeRect);
        }
    }
    
    if (pageIdx_ == ePage_Adc) {
        {
            constexpr int16_t w = 7;
            Rectange sliderRect = rect.RemoveFromLeft(w * 12);
            sliderRect.Reduced(0, 2);
            // draw adc bound line
            drawer.display.setColor(OledColorEnum::kOledWHITE);
            auto minAdc = static_cast<float>(Keyboard.GetMinAdcValue());
            auto adcUpPos = (Keyboard.GetADCUpValue() - minAdc) / (135.0f - minAdc);
            auto y = sliderRect.y + sliderRect.h * (1.0f - adcUpPos);
            drawer.display.drawHorizontalLine(sliderRect.x, y, sliderRect.w);
            adcUpPos = (Keyboard.GetADCDownValue() - minAdc) / (135.0f - minAdc);
            y = sliderRect.y + sliderRect.h * (1.0f - adcUpPos);
            drawer.display.drawHorizontalLine(sliderRect.x, y, sliderRect.w);
            // draw extra adc line
            sliderRect.w = 7;
            for (uint32_t i = 0; i < 12; ++i) {
                auto center = sliderRect.GetCenter();
                drawer.display.drawVerticalLine(center.x, sliderRect.y, sliderRect.h);
                auto pos = (Keyboard.GetAdcValue(i) - minAdc) / (135.0f - minAdc);
                auto y = sliderRect.y + sliderRect.h * (1.0f - pos);
                drawer.display.fillCircle(center.x, y, 2);
                sliderRect.Translated(w, 0);
            }
        }
        {
            auto frameRect = rect.WithHeight(12);
            auto t = rect.RemoveFromTop(12);
            drawer.display.setColor(OledColorEnum::kOledWHITE);
            drawer.display.FormatString(t.x, t.y, "弹起值:{}", Keyboard.GetADCUpValue());
            t = rect.RemoveFromTop(12);
            drawer.display.FormatString(t.x, t.y, "按下值:{}", Keyboard.GetADCDownValue());
            t = rect.RemoveFromTop(12);
            drawer.display.FormatString(t.x, t.y, "最小值:{}", Keyboard.GetMinAdcValue());
            frameRect.Translated(0, 12 * selectIdx);
            drawer.InverseFrame(frameRect);
        }
        Redraw(); // keep update
    }
    else if (pageIdx_ == ePage_TouchPad) {
        {
            auto padRect = rect.RemoveFromLeft(rect.h * 2 / 3);
            padRect.Reduced(2, 2);
            drawer.display.setColor(OledColorEnum::kOledWHITE);
            drawer.display.drawRect(padRect.x, padRect.y, padRect.w, padRect.h);
            auto center = padRect.GetCenter();
            drawer.display.drawVerticalLine(center.x, padRect.y, padRect.h);
            drawer.display.drawHorizontalLine(padRect.x, center.y, padRect.w);
            auto filterX = MidiManager.GetTouchPadX() * 2 - 1;
            auto filterY = MidiManager.GetTouchPadY() * 2 - 1;
            auto pointX = center.x + filterX * padRect.w / 2.0f;
            auto pointY = center.y - filterY * padRect.h / 2.0f;
            drawer.display.fillCircle(pointX, pointY, 2);
        }
        {
            auto sliderRect = rect.RemoveFromLeft(7);
            auto center = sliderRect.GetCenter();
            drawer.display.setColor(OledColorEnum::kOledWHITE);
            drawer.display.drawVerticalLine(center.x, sliderRect.y, sliderRect.h);
            float modWheelPos = bsp::MPR121.GetModWheelPos();
            auto y = sliderRect.y + sliderRect.h * (1.0f - modWheelPos);
            drawer.display.fillCircle(center.x, y, 2);

            sliderRect = rect.RemoveFromLeft(7);
            center = sliderRect.GetCenter();
            drawer.display.drawVerticalLine(center.x, sliderRect.y, sliderRect.h);
            float pitchWheelPos = bsp::MPR121.GetPitchBendPos();
            y = sliderRect.y + sliderRect.h * (1.0f - pitchWheelPos);
            drawer.display.fillCircle(center.x, y, 2);
        }
        {
            auto frameRect = rect.WithHeight(12);
            auto t = rect.RemoveFromTop(12);
            drawer.display.setColor(OledColorEnum::kOledWHITE);
            drawer.display.FormatString(t.x, t.y, "pad:{}ms", bsp::MPR121.GetPositionFilterTime());
            t = rect.RemoveFromTop(12);
            drawer.display.FormatString(t.x, t.y, "调制轮:{}ms", bsp::MPR121.GetModWheelFilterTime());
            t = rect.RemoveFromTop(12);
            drawer.display.FormatString(t.x, t.y, "弯音轮:{}ms", bsp::MPR121.GetPitchBendFilterTime());
            t = rect.RemoveFromTop(12);
            drawer.display.FormatString(t.x, t.y, "滤波.x:{:3f} y:{:3f}", MidiManager.GetTouchPadX(), MidiManager.GetTouchPadY());
            t = rect.RemoveFromTop(12);
            auto rawPos = bsp::MPR121.GetPosition();
            drawer.display.FormatString(t.x, t.y, "原始x:{:3f} y:{:3f}", rawPos.fX, rawPos.fY);
            t = rect.RemoveFromTop(12);
            drawer.display.FormatString(t.x, t.y, "flag:{:b}", bsp::MPR121.GetTounchData());
            frameRect.Translated(0, 12 * selectIdx);
            drawer.InverseFrame(frameRect);
        }
        Redraw(); // keep update
    }
    else if (pageIdx_ == ePage_TouchSlider) {
        {
            constexpr int16_t w = 7;
            Rectange sliderRect = rect.RemoveFromLeft(w * 12);
            sliderRect.Reduced(0, 1);
            sliderRect.w = 7;
            drawer.display.setColor(OledColorEnum::kOledWHITE);
            for (uint32_t i = 0; i < 12; ++i) {
                auto center = sliderRect.GetCenter();
                drawer.display.drawVerticalLine(center.x, sliderRect.y, sliderRect.h);
                auto pos = Keyboard.GetFingerPosition(i) / 127.0f;
                auto y = center.y + sliderRect.h * pos / 2;
                drawer.display.fillCircle(center.x, y, 2);
                sliderRect.Translated(w, 0);
            }
        }
        {
            auto frameRect = rect.WithHeight(12);
            auto t = rect.RemoveFromTop(12);
            drawer.display.setColor(OledColorEnum::kOledWHITE);
            drawer.display.FormatString(t.x, t.y, "滤波:{}", Keyboard.GetDataFilterAlphas());
            frameRect.Translated(0, 12 * selectIdx);
            drawer.InverseFrame(frameRect);
        }
        Redraw(); // keep update
    }
    else if (pageIdx_ == ePage_Screen) {
        {
            auto mirrorRect = rect.RemoveFromTop(12);
            drawer.display.setColor(OledColorEnum::kOledWHITE);
            drawer.display.drawString(mirrorRect.x, mirrorRect.y, "镜像");
            mirrorRect.RemovedFromLeft(drawer.display.getStringWidth("镜像"));
            auto x = mirrorRect.RemoveFromLeft(mirrorRect.w / 2);
            if (bsp::UC1638.xMirror_) {
                drawer.display.drawString(x.x, x.y, "左右镜像");
            }
            else {
                drawer.display.drawString(x.x, x.y, "左右正常");
            }
            if (selectIdx == eScreen_MirrorX) drawer.InverseFrame(x);
            if (bsp::UC1638.yMirror_) {
                drawer.display.drawString(mirrorRect.x, mirrorRect.y, "上下镜像");
            }
            else {
                drawer.display.drawString(mirrorRect.x, mirrorRect.y, "上下正常");
            }
            if (selectIdx == eScreen_MirrorY) drawer.InverseFrame(mirrorRect);
        }
        {
            auto bkRect = rect.RemoveFromTop(12);
            if (isBkLightOn_) {
                drawer.display.drawString(bkRect.x, bkRect.y, "开背光");
            }
            else {
                drawer.display.drawString(bkRect.x, bkRect.y, "关背光");
            }
            if (selectIdx == eScreen_BackLight) drawer.InverseFrame(bkRect);
        }
    }
    else if (pageIdx_ == ePage_Mpe) {
        auto t = rect.RemoveFromTop(12);
        drawer.display.FormatString(t.x, t.y, "MPE: {}", Keyboard.IsMPEEnabled());
        if (selectIdx == eMPE_MPE) drawer.InverseFrame(t);
        t.Translated(0, 12);
        drawer.display.FormatString(t.x, t.y, "Play: {}", MidiManager.IsPlay() ? "play" : "usb");
        if (selectIdx == eMPE_Play) drawer.InverseFrame(t);
        t.Translated(0, 12);
        drawer.display.FormatString(t.x, t.y, "弯音范围: {}半音", dsp::SynthParams.pitchBend.Get());
        if (selectIdx == eMPE_PitchBendRange) drawer.InverseFrame(t);
    }
}

void CSettings::OnSelect() {
    SetPageIdx(pageIdx_);

    ControlIO.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue) {
        auto numSelects = 0;
        switch (Settings.pageIdx_) {
        case ePage_Adc:
            numSelects = eAdcSelect_NumOptions;
            break;
        case ePage_TouchPad:
            numSelects = ePadSelect_NumOptions;
            break;
        case ePage_TouchSlider:
            numSelects = eSliderSelect_NumOptions;
            break;
        case ePage_Screen:
            numSelects = eScreen_NumOptions;
            break;
        case ePage_Mpe:
            numSelects = eMPE_NumOptions;
            break;
        }
        --numSelects;
        Settings.selectIdx = utli::Clamp(Settings.selectIdx + dvalue, 0, numSelects);
        Settings.Redraw();
    });
    ControlIO.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue) {
        switch (Settings.pageIdx_) {
        case ePage_Adc:
            switch (Settings.selectIdx) {
            case eAdcSelect_ADCDownValue:
                Keyboard.SetADCDownValue(utli::Clamp(Keyboard.GetADCDownValue() + dvalue, 0, 127));
                break;
            case eAdcSelect_ADCUpValue:
                Keyboard.SetADCUpValue(utli::Clamp(Keyboard.GetADCUpValue() + dvalue, 0, 127));
                break;
            case eAdcSelect_MinADCValue:
                Keyboard.SetMinAdcValue(utli::Clamp(Keyboard.GetMinAdcValue() + dvalue, 0, 127));
                break;
            }
            break;
        case ePage_TouchPad:
            switch (Settings.selectIdx) {
            case ePadSelect_ModWheelFilter:
                bsp::MPR121.SetModWheelFilter(bsp::MPR121.GetModWheelFilterTime() + dvalue * 10);
                break;
            case ePadSelect_PitchBendFilter:
                bsp::MPR121.SetPitchBendFilter(bsp::MPR121.GetPitchBendFilterTime() + dvalue * 10);
                break;
            case ePadSelect_TouchPadFilter:
                bsp::MPR121.SetPositionFilterTime(bsp::MPR121.GetPositionFilterTime() + dvalue * 10);
                break;
            }
            break;
        case ePage_TouchSlider:
            Keyboard.SetDataFilterAlphas(Keyboard.GetDataFilterAlphas() + dvalue / 500.0f);
            break;
        case ePage_Screen:
            switch (Settings.selectIdx) {
            case eScreen_BackLight:
                Settings.isBkLightOn_ = dvalue > 0;
                bsp::UC1638.SetBKlight(Settings.isBkLightOn_);
                break;
            case eScreen_MirrorX:
                bsp::UC1638.xMirror_ = dvalue > 0;
                bsp::UC1638.SetMirror(bsp::UC1638.xMirror_, bsp::UC1638.yMirror_);
                break;
            case eScreen_MirrorY:
                bsp::UC1638.yMirror_ = dvalue > 0;
                bsp::UC1638.SetMirror(bsp::UC1638.xMirror_, bsp::UC1638.yMirror_);
                break;
            }
            break;
        case ePage_Mpe:
            switch (Settings.selectIdx) {
            case eMPE_MPE:
                Keyboard.SetMPEEnable(dvalue > 0);
                break;
            case eMPE_Play:
                MidiManager.SetPlay(dvalue > 0);
                break;
            case eMPE_PitchBendRange:
                dsp::SynthParams.pitchBend.Add(dvalue, false);
                break;
            }
            break;
        }
        Settings.Redraw();
    });
    ControlIO.SetButtonCallback(bsp::ButtonId::kBtn0, [](bsp::ButtonEventArgs args) {
        if(args.IsAttack()) Settings.SetPageIdx(ePage_Adc);
    });
    ControlIO.SetButtonCallback(bsp::ButtonId::kBtn1, [](bsp::ButtonEventArgs args) {
        if(args.IsAttack()) Settings.SetPageIdx(ePage_TouchPad);
    });
    ControlIO.SetButtonCallback(bsp::ButtonId::kBtn2, [](bsp::ButtonEventArgs args) {
        if(args.IsAttack()) Settings.SetPageIdx(ePage_TouchSlider);
    });
    ControlIO.SetButtonCallback(bsp::ButtonId::kBtn3, [](bsp::ButtonEventArgs args) {
        if(args.IsAttack()) {
            Settings.SetPageIdx(ePage_Mpe);
        }
    });
    ControlIO.SetButtonCallback(bsp::ButtonId::kBtn4, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) Settings.SetPageIdx(ePage_Screen); 
    });
}

void CSettings::OnTimeTick(uint32_t msEscape) {
}

void CSettings::SetPageIdx(int8_t idx) {
    pageIdx_ = idx;
    auto numSelects = 0;
    switch (pageIdx_) {
    case ePage_Adc:
        numSelects = eAdcSelect_NumOptions;
        break;
    case ePage_TouchPad:
        numSelects = ePadSelect_NumOptions;
        break;
    case ePage_TouchSlider:
        numSelects = eSliderSelect_NumOptions;
        break;
    case ePage_Screen:
        numSelects = eScreen_NumOptions;
        break;
    case ePage_Mpe:
        numSelects = eMPE_NumOptions;
        break;
    }
    --numSelects;
    selectIdx = utli::Clamp(Settings.selectIdx, 0, numSelects);
    Redraw();
}
}