#include "Main.hpp"
#include "Styles.hpp"
#include "dsp/Synth.hpp"
#include "Reed.hpp"
#include "String.hpp"
#include "Bowed.hpp"
#include "Reverb.hpp"
#include "Preset.hpp"

enum SelectOptions {
    kOctave = 0,
    kInstrument,
    kNumSelectOptions
};

static constexpr int32_t kWhiteKeyIdxTable[7] = {0, 2, 4, 5, 7, 9, 11};
static constexpr int32_t kBlackKeyIdxTable[5] = {1, 3, 6, 8, 10};
static constexpr std::string_view kInstrumentNames[] {
    "拨弦", "木管", "弓弦"
};

namespace gui::internal {

void CMain::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    drawer.DrawTitleBar("main");
    drawer.DrawBottomOptions(0, "模型");
    drawer.DrawBottomOptions(1, "混响");
    drawer.DrawBottomOptions(2, "这是啥");

    auto rect = drawer.display.GetDrawAera();
    rect.RemovedFromTop(12);
    rect.RemovedFromBottom(12);
    {
        auto line = rect.RemoveFromTop(12);
        auto octaveText = line.RemoveFromLeft(line.w / 2);
        drawer.display.setColor(OledColorEnum::kOledWHITE);
        drawer.display.FormatString(octaveText.x, octaveText.y, "八度: {}", octave);
        if (selectIdx == kOctave) drawer.InverseFrame(octaveText);
    
        drawer.display.FormatString(line.x, line.y, "模型: {}", kInstrumentNames[static_cast<int>(dsp::Synth.GetInstrument())]);
        if (selectIdx == kInstrument) drawer.InverseFrame(line);
    }

    // draw keyboard
    {
        auto keyboard = rect.RemoveFromTop(60);
        auto whiteKeyAera = keyboard.WithWidth(keyboard.w / 7);
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
}

void CMain::OnSelect() {
    auto& io = bsp::ControlIO;
    io.SetButtonCallback(bsp::ButtonId::kBtn0, [](auto state) {
        if (state.IsAttack()) {
            switch (dsp::Synth.GetInstrument()) {
            case dsp::CSynth::Instrument::String:
                GuiDispatch.SetObj(String);
                break;
            case dsp::CSynth::Instrument::Bow:
                GuiDispatch.SetObj(Bowed);
                break;
            case dsp::CSynth::Instrument::Reed:
                GuiDispatch.SetObj(Reed);
                break;
            }
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn1, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(Reverb);
        }
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn2, [](bsp::ButtonEventArgs args) {
        if (args.IsAttack()) {
            GuiDispatch.SetObj(Preset);
        }
    });
    io.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue) {
        auto& main = Main;
        main.selectIdx += dvalue;
        main.selectIdx = std::clamp(main.selectIdx, 0, kNumSelectOptions - 1);
        Main.Redraw();
    });
    io.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue) {
        auto& main = Main;
        switch (main.selectIdx) {
        case kOctave:
            main.octave += dvalue;
            main.octave = std::clamp(main.octave, 0, 9);
            Main.Redraw();
            break;
        case kInstrument: {
            int32_t instrument = static_cast<int>(dsp::Synth.GetInstrument());
            instrument += dvalue;
            instrument = std::clamp(instrument, 0, static_cast<int32_t>(dsp::CSynth::Instrument::kNumInstruments) - 1);
            Main.Redraw();
            dsp::Synth.SetInstrument(static_cast<dsp::CSynth::Instrument>(instrument));
        }
            break;
        }
    });
}

void CMain::AddOctave(int i) {
    octave += i;
    octave = std::clamp(octave, 0, 9);
    Main.Redraw();
}
}