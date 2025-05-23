#pragma once
#include <cstdint>
#include <concepts>
#include <usf/usf.hpp>

namespace bsp {

enum class LedEnum {
    Red,
    Green,
    Blue  
};

struct LedErrorSequence {
    uint32_t numShort;
    uint32_t shortMs;
    uint32_t numLong;
    uint32_t longMs;
};
inline static constexpr struct CLedErrorSequencePresets {
    LedErrorSequence InterruptNotHandle = {
        .numShort = 1,
        .shortMs = 500,
        .numLong = 1,
        .longMs = 1000,
    };
    LedErrorSequence HardFault = {
        .numShort = 1,
        .shortMs = 500,
        .numLong = 2,
        .longMs = 1000,
    };
    LedErrorSequence RTOSAssert = {
        .numShort = 1,
        .shortMs = 500,
        .numLong = 3,
        .longMs = 1000,
    };
    LedErrorSequence CloclError = {
        .numShort = 1,
        .shortMs = 500,
        .numLong = 4,
        .longMs = 1000,
    };
    LedErrorSequence DeviceError = {
        .numShort = 2,
        .shortMs = 500,
        .numLong = 1,
        .longMs = 1000,
    };
    LedErrorSequence PeriphClockError = {
        .numShort = 3,
        .shortMs = 500,
        .numLong = 1,
        .longMs = 1000,
    };
    LedErrorSequence AppError = {
        .numShort = 2,
        .shortMs = 500,
        .numLong = 2,
        .longMs = 1000,
    };
} LedErrorSequencePreset;

class DebugIO {
public:
    static constexpr uint32_t kMaxPrintfString = 256;

    void Init();
    void InitLed();
    void PrintStackAndRegisters();

    // ---------------------------------------- Poll Serial ----------------------------------------
    template <typename... Args> USF_CPP14_CONSTEXPR
    void Write(usf::StringView fmt, Args&&... args) {
        char printBuffer[kMaxPrintfString];
        usf::StringSpan span = usf::format_to(usf::StringSpan(printBuffer, kMaxPrintfString), fmt, std::forward<Args>(args)...);
        WriteSerialPoll(span);
    }

    template <typename... Args> USF_CPP14_CONSTEXPR
    void WriteLine(usf::StringView fmt, Args&&... args) {
        char printBuffer[kMaxPrintfString];
        usf::StringSpan span = usf::format_to(usf::StringSpan(printBuffer, kMaxPrintfString), fmt, std::forward<Args>(args)...);
        WriteSerialPoll(span);
        NewLine();
    }

    void NewLine();

    // ---------------------------------------- RTOS Serial ----------------------------------------
    void StartRTOSIO();
    template <typename... Args> USF_CPP14_CONSTEXPR
    void XWrite(usf::StringView fmt, Args&&... args) {
        usf::StringSpan span = usf::format_to(GetRTOSPrintBuffer(), fmt, std::forward<Args>(args)...);
        XWriteSerial(span, false);
    }

    template <typename... Args> USF_CPP14_CONSTEXPR
    void XWriteLine(usf::StringView fmt, Args&&... args) {
        usf::StringSpan span = usf::format_to(GetRTOSPrintBuffer(), fmt, std::forward<Args>(args)...);
        XWriteSerial(span, true);
    }

    // ---------------------------------------- Time ----------------------------------------
    uint32_t GetCpuSpeed();
    void Wait(uint32_t ms);
    void Wait(uint32_t ms, uint32_t cpuSpeed);
    void HalWait(uint32_t ms);

    // ---------------------------------------- Debug ----------------------------------------
    [[noreturn]] void FlashErrorLight(uint32_t ms);
    [[noreturn]] void FlashErrorLightSequence(LedErrorSequence sequence);
    void SetLed(bool r, bool g, bool b);
    void ToggleLed(LedEnum led);
    void SetLed(LedEnum led, bool on);
private:
    usf::StringSpan GetRTOSPrintBuffer();
    void WriteSerialPoll(usf::StringSpan span);
    void XWriteSerial(usf::StringSpan span, bool newLine);

    bool rtosInit_ = false;
};

extern DebugIO Debug;

}