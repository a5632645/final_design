#include <raylib.h>
#include "oled/OLEDDisplay.h"
#include "bsp/ControlIO.hpp"
#include "gui/GuiDispatch.hpp"
#include "dsp/Synth.hpp"
#include "RtMidi.h"
#include "gui/obj/Main.hpp"
#include <semaphore>
#include <format>
#include "dsp/MidiManager.hpp"

// static NoteQueue noteQueue;
static float auxBuffer[512]{};
static float dacBuffer[512]{};
static void DAC_Callback(void* buffer, uint32_t size) {
    auto& synth = dsp::Synth;
    {
        MidiManager.BackupState();
        auto diff = MidiManager.GetDiff0();
        while (diff) {
            auto note = __builtin_ctzl(diff);
            auto velocity = MidiManager.GetLastVelocity(note);
            auto channel = MidiManager.GetLastChannelOfNote(note);
            if (velocity) {
                synth.NoteOn(channel, note, velocity);
            } else {
                synth.NoteOff(note);
            }
            diff &= ~(1ULL << note);
        }
        diff = MidiManager.GetDiff1();
        while (diff) {
            auto note = __builtin_ctzl(diff);
            auto velocity = MidiManager.GetLastVelocity(note + 32);
            auto channel = MidiManager.GetLastChannelOfNote(note + 32);
            if (velocity) {
                synth.NoteOn(channel, note + 32, velocity);
            } else {
                synth.NoteOff(note + 32);
            }
            diff &= ~(1ULL << note);
        }
        diff = MidiManager.GetDiff2();
        while (diff) {
            auto note = __builtin_ctzl(diff);
            auto velocity = MidiManager.GetLastVelocity(note + 64);
            auto channel = MidiManager.GetLastChannelOfNote(note + 64);
            if (velocity) {
                synth.NoteOn(channel, note + 64, velocity);
            } else {
                synth.NoteOff(note + 64);
            }
            diff &= ~(1ULL << note);
        }
        diff = MidiManager.GetDiff3();
        while (diff) {
            auto note = __builtin_ctzl(diff);
            auto velocity = MidiManager.GetLastVelocity(note + 96);
            auto channel = MidiManager.GetLastChannelOfNote(note + 96);
            if (velocity) {
                synth.NoteOn(channel, note + 96, velocity);
            } else {
                synth.NoteOff(note + 96);
            }
            diff &= ~(1ULL << note);
        }
    }
    {
        std::lock_guard lock { dsp::gSafeCallback };
        dsp::gSafeCallback.HandleDirtyCallbacks();
    }

    synth.Process(std::span{dacBuffer, size}, std::span{auxBuffer, size});
    struct Wtf {
        float left;
        float rigth;
    };
    Wtf* buf = static_cast<Wtf*>(buffer);
    for (int i = 0; i < size; i++) {
        buf[i].left = dacBuffer[i];
        buf[i].rigth = auxBuffer[i];
    }
}

static void CheckComputerMidiKeyboard() {
    static constexpr std::array kKeys {
        KEY_A,
        KEY_W,
        KEY_S,
        KEY_E,
        KEY_D,
        KEY_F,
        KEY_T,
        KEY_G,
        KEY_Y,
        KEY_H,
        KEY_U,
        KEY_J
    };
    if (IsKeyPressed(KEY_KP_ADD)) {
        gui::Main.AddOctave(1);
    }
    if (IsKeyPressed(KEY_KP_SUBTRACT)) {
        gui::Main.AddOctave(-1);
    }

    {
        for (int i = 0; i < kKeys.size(); i++) {
            if (IsKeyPressed(kKeys[i])) {
                gui::Main.NoteOn(i);
                MidiManager.NoteOn(0, gui::Main.octave * 12 + i, 127);
            }
            if (IsKeyReleased(kKeys[i])) {
                gui::Main.NoteOff(i);
                MidiManager.NoteOff(0, gui::Main.octave * 12 + i);
            }
        }
    }
}

static void MidiCallback(double timeStamp, std::vector<unsigned char>* message, void *userData) {
    auto& midi = *message;
    auto& mgr = MidiManager;

    auto channel = midi.at(0) & 0x0f;
    switch (midi.at(0) >> 4) {
    case 0x8:
        mgr.NoteOff(channel, midi.at(1));
        gui::Main.NoteOff(midi.at(1) % 12);
        break;
    case 0x9:
        mgr.NoteOn(channel, midi.at(1), midi.at(2));
        gui::Main.NoteOn(midi.at(1) % 12);
        break;
    case 0xb:
        mgr.SetCC(channel, midi.at(1), midi.at(2));
        break;
    case 0xd:
        mgr.SetPressure(channel, midi.at(1));
        break;
    case 0xe:
        mgr.SetPitchBend(channel, midi.at(2), midi.at(1));
        break;
    }
}

OLEDDisplay display;
OLEDRGBColor buffer[OLEDDisplay::kBufferSize];

int main(void) {
    InitWindow(OLEDDisplay::kWidth, OLEDDisplay::kHeight, "test");
    SetTargetFPS(60);
    InitAudioDevice();
    auto stream = LoadAudioStream(48000, 512, 2);
    SetAudioStreamCallback(stream, DAC_Callback);
    dsp::Synth.Init(48000);
    dsp::gSafeCallback.MarkAll();
    PlayAudioStream(stream);
    
    bsp::CControlIO io;
    io.Init();

    display.SetDisplayBuffer(buffer);
    gui::GuiDispatch.Init();

    RtMidiIn midi{ RtMidiIn::Api::WINDOWS_MM };
    try {
        midi.setCallback(&MidiCallback, nullptr);
        midi.openPort(0);
    }
    catch(...) {
        std::cout << "Failed to open midi port" << std::endl;
    }

    auto lastTime = GetTime();
    while (!WindowShouldClose()) {
        io.WaitForNextEvent();
        io.ProcessButtonEvents();
        io.ProcessEncoderEvents();
        CheckComputerMidiKeyboard();

        auto now = GetTime();
        auto msEscape = (now - lastTime) * 1000;
        lastTime = now;
        bool needRedraw = gui::GuiDispatch.Update(msEscape);

        BeginDrawing();
        auto* buffer = display.getDisplayBuffer();
        // for (int y = 0; y < 96 / 8; y++) {
        //     for (int x = 0; x < 240; x++) {
        //         auto index = x + y * 240;
        //         auto realY = y * 8;
        //         auto byte = buffer[index];
        //         for (int i = 0; i < 8; i++) {
        //             if (byte & (1 << i)) {
        //                 DrawPixel(x, realY + i, WHITE);
        //             }
        //             else {
        //                 DrawPixel(x, realY + i, BLACK);
        //             }
        //         }
        //     }
        // }
        for (int y = 0; y < display.kHeight; ++y) {
            for (int x = 0; x < display.kWidth; ++x) {
                auto w = buffer[y * display.kWidth + x].ToRGB888();
                Color color;
                color.r = w.r;
                color.b = w.b;
                color.g = w.g;
                color.a = 0xff;
                DrawPixel(x, y, color);
            }
        }
        EndDrawing();
    }

    UnloadAudioStream(stream);
    CloseAudioDevice();
    CloseWindow();
}