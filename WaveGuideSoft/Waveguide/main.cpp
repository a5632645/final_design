#include "bsp/MCU.hpp"
#include "bsp/DebugIO.hpp"
#include "bsp/UC1638.hpp"
#include "bsp/PCM5102.hpp"
#include "bsp/USBMidi.hpp"
#include "bsp/ControlIO.hpp"
#include "bsp/MPR121.hpp"
#include "bsp/Keyboard.hpp"

#include "gui/GuiDispatch.hpp"
#include "gui/obj/Main.hpp"
#include "gui/obj/Scope.hpp"

#include "dsp/Synth.hpp"
#include "utli/Clamp.hpp"
#include "utli/Map.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "MemAttributes.hpp"
#include "SystemHook.hpp"
#include "MidiManager.hpp"
#include <cmath>

using bsp::Debug;

// --------------------------------------------------------------------------------
// DAC
// --------------------------------------------------------------------------------
static StaticTask_t dacTaskBuffer;
MEM_NOINIT_SRAMD1 static StackType_t dacTaskStack[8192];
static float synthBuffer[bsp::CPCM5102::kBlockSize];
static float auxSynthBuffer[bsp::CPCM5102::kBlockSize];
static float volume_ = 1.0f;
static void DACTaskInit() {
    APP_LOG("main", "start dac");
    dsp::SynthParams.volume.SetCallback([] {
        auto db = dsp::SynthParams.volume.Get();
        volume_ = std::pow(10.0f, db / 20.0f);
    });
    xTaskCreateStatic(
        [](void*) {
            using bsp::PCM5102;
            PCM5102.Init();
            dsp::Synth.Init(PCM5102.kSampleRate);
            MidiManager.Init(PCM5102.kSampleRate / PCM5102.kBlockSize);
            dsp::gSafeCallback.MarkAll();
            PCM5102.Start();
            for (;;) {
                auto block = PCM5102.GetNextBlock();
                TickType_t tickBegin = xTaskGetTickCount();

                // handle datas
                MidiManager.Lock();
                MidiManager.BackupState();
                MidiManager.Unlock();
                
                { // handle notes
                    auto diff = MidiManager.GetDiff0();
                    while (diff) {
                        auto idx = __builtin_ctzl(diff);
                        if (MidiManager.GetVelocity(idx) > 0) {
                            dsp::Synth.NoteOn(MidiManager.GetChannelOfNote(idx), idx, MidiManager.GetVelocity(idx));
                        }
                        else {
                            dsp::Synth.NoteOff(idx);
                        }
                        diff &= ~(MidiManager.kOne << idx);
                    }
                    diff = MidiManager.GetDiff1();
                    while (diff) {
                        auto idx = __builtin_ctzl(diff);
                        auto note = idx + 32;
                        if (MidiManager.GetVelocity(note) > 0) {
                            dsp::Synth.NoteOn(MidiManager.GetChannelOfNote(note), note, MidiManager.GetVelocity(note));
                        }
                        else {
                            dsp::Synth.NoteOff(note);
                        }
                        diff &= ~(MidiManager.kOne << idx);
                    }
                    diff = MidiManager.GetDiff2();
                    while (diff) {
                        auto idx = __builtin_ctzl(diff);
                        auto note = idx + 64;
                        if (MidiManager.GetVelocity(note) > 0) {
                            dsp::Synth.NoteOn(MidiManager.GetChannelOfNote(note), note, MidiManager.GetVelocity(note));
                        }
                        else {
                            dsp::Synth.NoteOff(note);
                        }
                        diff &= ~(MidiManager.kOne << idx);
                    }
                    diff = MidiManager.GetDiff3();
                    while (diff) {
                        auto idx = __builtin_ctzl(diff);
                        auto note = idx + 96;
                        if (MidiManager.GetVelocity(note) > 0) {
                            dsp::Synth.NoteOn(MidiManager.GetChannelOfNote(note), note, MidiManager.GetVelocity(note));
                        }
                        else {
                            dsp::Synth.NoteOff(note);
                        }
                        diff &= ~(MidiManager.kOne << idx);
                    }
                }

                // handle parameter changes
                dsp::gSafeCallback.HandleDirtyCallbacks();

                // process audio
                dsp::Synth.Process(synthBuffer, auxSynthBuffer);

                // mix in
                for (uint32_t i = 0; i < bsp::CPCM5102::kBlockSize; i++) {
                    synthBuffer[i] *= volume_;
                    auto left = synthBuffer[i] * 16384;
                    left = dsp::ClampUncheck(left, -32768, 32767);
                    auto iLeft = static_cast<int16_t>(left);
                    block[i].left = iLeft;
                    auxSynthBuffer[i] *= volume_;
                    auto right = auxSynthBuffer[i] * 16384;
                    right = dsp::ClampUncheck(right, -32768, 32767);
                    auto iRight = static_cast<int16_t>(right);
                    block[i].right = iRight;
                }
                gui::Scope.Push(std::span{synthBuffer, bsp::CPCM5102::kBlockSize});

                TickType_t tickEnd = xTaskGetTickCount();
                gui::Main.SetDacTaskMs(pdTICKS_TO_MS(tickEnd - tickBegin));
            }
        },
        "DAC",
        std::size(dacTaskStack),
        nullptr,
        3,
        dacTaskStack,
        &dacTaskBuffer
    );
}

// --------------------------------------------------------------------------------
// USB Midi Task
// --------------------------------------------------------------------------------
static StaticTask_t usbMidiTaskBuffer;
MEM_NOINIT_SRAMD1 static StackType_t usbMidiTaskStack[1024];
static void USBMidiTaskInit() {
    APP_LOG("main", "start usb");
    xTaskCreateStatic(
        [](void*) {
            using bsp::USBMidi;

            USBMidi.Init();
            USBMidi.SetMidiReciveIRQ([](std::span<const bsp::MidiEvent> events) {
                for (const auto& e : events) {
                    switch (e.GetType()) {
                    case bsp::MidiEvent::Type::kNoteOn:
                        MidiManager.NoteOn(e.GetChannel(), e.GetNote(), e.GetVelocity());
                        gui::Main.NoteOn(e.GetNote());
                        break;
                    case bsp::MidiEvent::Type::kNoteOff:
                        MidiManager.NoteOff(e.GetChannel(), e.GetNote());
                        gui::Main.NoteOff(e.GetNote());
                        break;
                    case bsp::MidiEvent::Type::kPitchBend:
                        MidiManager.SetTouchSliderPos(e.GetChannel(), (e.GetPitchBend() - 8192) / 8192.0f);
                        break;
                    case bsp::MidiEvent::Type::kPressure:
                        MidiManager.SetPressure(e.GetChannel(), e.data2);
                        break;
                    case bsp::MidiEvent::Type::kCC:
                        MidiManager.SetCC(e.GetChannel(), e.data2, e.data3);
                        break;
                    default:
                        break;
                    }
                }
            });
            for (;;) {
                USBMidi.SendData();
            }
        },
        "USBMidi",
        std::size(usbMidiTaskStack),
        nullptr,
        1,
        usbMidiTaskStack,
        &usbMidiTaskBuffer
    );
}

// --------------------------------------------------------------------------------
// LCD
// --------------------------------------------------------------------------------
static StaticTask_t lcdTaskBuffer;
MEM_NOINIT_SRAMD1 static StackType_t lcdTaskStack[1024];
static void LCDTaskInit() {
    APP_LOG("main", "start lcd");
    xTaskCreateStatic(
        [](void*) {
            using bsp::UC1638;
            using bsp::ControlIO;
            using gui::GuiDispatch;

            ControlIO.Init();

            UC1638.Init();
            UC1638.SetBKlight(true);
            UC1638.HardReset();
            UC1638.PowerUpSequence();

            GuiDispatch.Init();
            TickType_t lastTick = xTaskGetTickCount();
            for (;;) {
                ControlIO.WaitForNextEvent();
                ControlIO.ProcessButtonEvents();
                ControlIO.ProcessEncoderEvents();

                TickType_t now = xTaskGetTickCount();
                auto msEscape = pdTICKS_TO_MS(now - lastTick);
                lastTick = now;
                auto shouldSendBuffer = GuiDispatch.Update(msEscape);
                if (shouldSendBuffer) {
                    UC1638.UpdateScreen();
                }
            }
        },
        "LCD",
        std::size(lcdTaskStack),
        nullptr,
        1,
        lcdTaskStack,
        &lcdTaskBuffer
    );
}

// --------------------------------------------------------------------------------
// MPR121
// --------------------------------------------------------------------------------
static StaticTask_t mpr121TaskBuffer;
MEM_NOINIT_SRAMD1 static StackType_t mpr121TaskStack[1024];
static void MPR121TaskInit() {
    APP_LOG("main", "start mpr121");
    xTaskCreateStatic(
        [](void*) {
            using bsp::MPR121;

            constexpr auto kInterval = 20;
            constexpr auto kFreq = 1000 / kInterval;
            MPR121.Init(kFreq);
            TickType_t lastTick = xTaskGetTickCount();
            for (;;) {
                MPR121.UpdateData();
                MidiManager.Lock();
                MidiManager.SetIsTouched(MPR121.GetTounchData() != 0);
                auto pos = MPR121.GetPosition();
                MidiManager.SetTouchPad(pos.fX, pos.fY);

                uint8_t pitchBendChannel = 0;
                if (bsp::Keyboard.IsMPEEnabled()) {
                    pitchBendChannel = 15;
                }
                if (MPR121.IsPitchBendTouched()) {
                    bsp::USBMidi.WritePitchBend(pitchBendChannel, utli::Map(0, 0x3fff, 0.0f, 1.0f, MPR121.GetPitchBendPos()));
                }
                if (MidiManager.IsPlay()) {
                    MidiManager.SetTouchSliderPos(pitchBendChannel, 2.0f *MPR121.GetPitchBendPos() - 1.0f);
                }
                if (MPR121.IsModWheelTouched()) {
                    bsp::USBMidi.WriteCC(pitchBendChannel, 1, utli::Map(0, 127, 0.0f, 1.0f, MPR121.GetModWheelPos()));
                }
                if (MidiManager.IsPlay()) {
                    MidiManager.SetCC(pitchBendChannel, 1, MPR121.GetModWheelPos() * 127);
                }

                MidiManager.Unlock();
                vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(20));
            }
        },
        "MPR121",
        std::size(mpr121TaskStack),
        nullptr,
        1,
        mpr121TaskStack,
        &mpr121TaskBuffer
    );
}

// --------------------------------------------------------------------------------
// keyboard
// --------------------------------------------------------------------------------
static StaticTask_t keyboardTask;
MEM_NOINIT_SRAMD1 static StackType_t keyboardTaskStack[1024];
static void KeyboardTaskInit() {
    APP_LOG("main", "start keyboard");
    xTaskCreateStatic(
        [](void*) {
            using bsp::Keyboard;
            using gui::Main;
            using bsp::USBMidi;

            static constexpr uint32_t timeInterval = 10;
            static constexpr uint32_t dataRate = 1000 / timeInterval;

            Keyboard.Init(dataRate);
            Keyboard.SetDataFilterAlphas(0.9f);
            Keyboard.SetKeyUpCallback([](uint8_t keyIdx) {
                Main.NoteOff(keyIdx);
                auto note = keyIdx + 12 * Main.octave;
                auto channel = MidiManager.NoteOff(note);
                if (!Keyboard.IsMPEEnabled()) {
                    channel = 0;
                }
                USBMidi.WriteNoteOff(channel, note, 0);
                gui::Main.NoteOff(keyIdx);
            });
            Keyboard.SetKeyDownCallback([](uint8_t keyIdx) {
                Main.NoteOn(keyIdx);
                auto note = keyIdx + 12 * Main.octave;
                auto channel = MidiManager.NoteOn(note, 120);
                if (!Keyboard.IsMPEEnabled()) {
                    channel = 0;
                }
                USBMidi.WriteNoteOn(channel, note, 120);
                gui::Main.NoteOn(keyIdx);
            });

            TickType_t tmp = xTaskGetTickCount();
            for (;;) {
                Keyboard.UpdateData();
                taskENTER_CRITICAL();
                Keyboard.ProcessData();

                for (uint32_t i = 0; i < Keyboard.kNumKeys; ++i) {
                    if (!Keyboard.IsKeyPressed(i)) continue;

                    if (MidiManager.IsPlay()) {
                        if (Keyboard.IsMPEEnabled()) {
                            MidiManager.SetPressure(i, Keyboard.GetPressure(i));
                            MidiManager.SetTouchSliderPos(i, Keyboard.GetFingerPosition(i));
                        }
                        else {
                            auto v = MidiManager.GetPressure(0);
                            auto u = Keyboard.GetPressure(i) / 127.0f;
                            if (u > v) {
                                MidiManager.SetPressure(0, Keyboard.GetPressure(i));
                            }
                        }
                    }

                    if (Keyboard.IsMPEEnabled()) {
                        auto note = i + 12 * Main.octave;
                        if (MidiManager.kInvalidChannel != MidiManager.GetChannelOfNote(note)) {
                            USBMidi.WritePitchBendScaled(MidiManager.GetChannelOfNote(note), Keyboard.GetFingerPosition(i));
                            USBMidi.WriteChannelPressure(MidiManager.GetChannelOfNote(note), Keyboard.GetPressure(i));
                        }
                    }
                    else {
                        // only send polyaftertouch
                        auto note = i + 12 * Main.octave;
                        USBMidi.WritePolyAfterTouch(0, note, Keyboard.GetPressure(i));
                    }
                }
                taskEXIT_CRITICAL();
                USBMidi.NotifySend(false);
                vTaskDelayUntil(&tmp, pdMS_TO_TICKS(timeInterval));
            }
        },
        "keyboard",
        std::size(keyboardTaskStack),
        nullptr,
        2,
        keyboardTaskStack,
        &keyboardTask
    );
}

// --------------------------------------------------------------------------------
// main
// --------------------------------------------------------------------------------
void AppMain(void*) {
    Debug.StartRTOSIO();

    USBMidiTaskInit();
    KeyboardTaskInit();
    LCDTaskInit();
    DACTaskInit();
    MPR121TaskInit();

    vTaskDelete(nullptr);
}

int main() {
    bsp::MCU.Init();
    bsp::MCU.InitD1Memory();
    bsp::MCU.InitITCRAM();
    bsp::MCU.MpuInitDmaMemory();

    Debug.Init();
    Debug.SetLed(false, false, false);

    xTaskCreate(AppMain, "AppMain", 1024, nullptr, 1, nullptr);
    vTaskStartScheduler();
    return 0;
}
