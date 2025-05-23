#include "Main.hpp"
#include "Styles.hpp"
#include "utli/Clamp.hpp"
#include "dsp/Synth.hpp"
#include "gui/obj/Settings.hpp"
#include "gui/obj/Reverb.hpp"
#include "gui/obj/String.hpp"
#include "gui/obj/Preset.hpp"
#include "gui/obj/Reed.hpp"
#include "gui/obj/Bowed.hpp"
#include "gui/obj/Scope.hpp"
#include "bsp/PCM5102.hpp"

enum SelectOptions {
    kOctave = 0,
    kInstrument,
    kVolume,
    kNumSelectOptions
};

static constexpr int32_t kWhiteKeyIdxTable[7] = {0, 2, 4, 5, 7, 9, 11};
static constexpr int32_t kBlackKeyIdxTable[5] = {1, 3, 6, 8, 10};
static constexpr std::string_view kInstrumentNames[] {
    "拨弦", "木管", "弓弦"
};

void gui::CMain::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    drawer.DrawTitleBar("main");

    auto rect = drawer.display.GetDrawAera();
    {
        drawer.display.setColor(OledColorEnum::kOledWHITE);
        auto textRect = drawer.GetBottomRect(0);
        static constexpr std::string_view kMeunTexts[] {
            "设置", "波形", "模型", "混响", "预设"
        };
        for (auto s : kMeunTexts) {
            drawer.display.drawString(textRect.x, textRect.y, s);
            textRect.Translated(textRect.w, 0);
        }
    }

    rect.RemovedFromTop(12);
    rect.RemovedFromBottom(12);
    {
        auto line = rect.RemoveFromTop(12);
        auto t = line.w / 3;
        auto octaveText = line.RemoveFromLeft(t);
        drawer.display.setColor(OledColorEnum::kOledWHITE);

        drawer.display.FormatString(octaveText.x, octaveText.y, "八度:{}", octave);
        if (selectIdx_ == kOctave) drawer.InverseFrame(octaveText);
    
        octaveText = line.RemoveFromLeft(t);
        drawer.display.FormatString(octaveText.x, octaveText.y, "模型:{}", kInstrumentNames[static_cast<uint32_t>(dsp::Synth.GetInstrument())]);
        if (selectIdx_ == kInstrument) drawer.InverseFrame(octaveText);
        drawer.display.FormatString(line.x, line.y, "音量:{}dB", dsp::SynthParams.volume.Get());
        if (selectIdx_ == kVolume) drawer.InverseFrame(line);
    }

    {
        // draw keyboard
        auto keyboard = rect.RemoveFromTop(60);
        auto whiteKeyAera = keyboard.WithWidth(20);
        auto blackKeyAera = Rectange{whiteKeyAera}.RemoveFromTop(whiteKeyAera.h * 2 / 3);
        blackKeyAera.Translated(blackKeyAera.w / 2, 0);
        for (int i = 0; i < 7; i++) {
            auto bkcolor = OledColorEnum::kOledWHITE;
            auto framecolor = OledColorEnum::kOledBLACK;
            auto keyIdx = kWhiteKeyIdxTable[i];
            if (keyPressState & (1 << keyIdx)) {
                std::swap(bkcolor, framecolor);
            }
            drawer.display.setColor(bkcolor);
            drawer.display.fillRect(whiteKeyAera.x + i * whiteKeyAera.w, whiteKeyAera.y, whiteKeyAera.w, whiteKeyAera.h);
            drawer.display.setColor(framecolor);
            drawer.display.drawRect(whiteKeyAera.x + i * whiteKeyAera.w, whiteKeyAera.y, whiteKeyAera.w, whiteKeyAera.h);
            whiteKeyAera.Translated(2, 0);
        }
    
        for (int i = 0; i < 5; i++) {
            auto bkcolor = OledColorEnum::kOledWHITE;
            auto framecolor = OledColorEnum::kOledBLACK;
            auto keyIdx = kBlackKeyIdxTable[i];
            if (keyPressState & (1 << keyIdx)) {
                std::swap(bkcolor, framecolor);
            }
            drawer.display.setColor(bkcolor);
            drawer.display.fillRect(blackKeyAera.x + i * blackKeyAera.w, blackKeyAera.y, blackKeyAera.w, blackKeyAera.h);
            drawer.display.setColor(framecolor);
            drawer.display.drawRect(blackKeyAera.x + i * blackKeyAera.w, blackKeyAera.y, blackKeyAera.w, blackKeyAera.h);
            if (i != 1) {
                blackKeyAera.Translated(2, 0);
            }
            else {
                blackKeyAera.Translated(2 + blackKeyAera.w, 0);
            }
        }
    }

    { // draw other infomations
        rect.Reduced(1, 1);
        drawer.display.setColor(OledColorEnum::kOledWHITE);
        drawer.display.FormatString(rect.x, rect.y, "dac:{}ms", dacTaskTicks_);
    }
}

void gui::CMain::OnSelect() {
    auto& io = bsp::ControlIO;
    io.SetButtonCallback(bsp::ButtonId::kBtn2, [](auto state) {
        if (state.IsAttack()) {
            switch (dsp::Synth.GetInstrument()) {
            case dsp::Instrument::String:
                GuiDispatch.SetObj(String);
                break;
            case dsp::Instrument::Bow:
                GuiDispatch.SetObj(Bowed);
                break;
            case dsp::Instrument::Reed:
                GuiDispatch.SetObj(Reed);
                break;
            }
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn3, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(Reverb);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn4, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(Preset);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn0, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(Settings);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn1, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(Scope);
        }
    });
    io.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue) {
        auto& main = Main;
        main.selectIdx_ += dvalue;
        main.selectIdx_ = std::clamp<int32_t>(main.selectIdx_, 0, kNumSelectOptions - 1);
        Main.Redraw();
    });
    io.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue) {
        auto& main = Main;
        switch (main.selectIdx_) {
        case kOctave:
            Main.SetOctave(Main.octave + dvalue);
            Main.Redraw();
            break;
        case kInstrument: {
            int32_t instrument = static_cast<int>(dsp::Synth.GetInstrument());
            instrument += dvalue;
            instrument = std::clamp<int32_t>(instrument, 0, static_cast<int32_t>(dsp::Instrument::kNumInstruments) - 1);
            Main.Redraw();
            dsp::Synth.SetInstrument(static_cast<dsp::Instrument>(instrument));
            break;
        }
        case kVolume:
            dsp::SynthParams.volume.Add(dvalue, false);
            Main.Redraw();
            break;
        }
    });
}

void gui::CMain::SetDacTaskMs(uint32_t ms) {
    if (dacTaskTicks_ != ms) {
        dacTaskTicks_ = ms;
        Redraw();
    }
}
