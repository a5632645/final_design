#include "FreeRTOS.h"
#include "task.h"
#include "bsp/DebugIO.hpp"
#include "SystemHook.hpp"

using namespace bsp;

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    Debug.WriteLine("{} stack overflow", pcTaskName);
    Debug.FlashErrorLightSequence(LedErrorSequencePreset.RTOSAssert);
}

void RTOS_Assert(const char* file, int line) {
    Debug.WriteLine("RTOS assert at {}:{}", file, line);
    Debug.FlashErrorLightSequence(LedErrorSequencePreset.RTOSAssert);
}

void Error_Handler(void) {
    Debug.WriteLine("error handler");
    Debug.FlashErrorLightSequence(LedErrorSequencePreset.DeviceError);
}

void hook::DeviceError(const char* tag, const char* msg, const char* file, int line) {
    Debug.WriteLine("[error]: {}: {} at {}:{}", tag, msg, file, line);
    Debug.FlashErrorLightSequence(LedErrorSequencePreset.DeviceError);
}

void hook::DeviceErrorCode(const char* tag, const char* msg, int32_t code, const char* file, int line) {
    Debug.WriteLine("[error]: {}: {} code: {} at {}:{}", tag, msg, code, file, line);
    Debug.FlashErrorLightSequence(LedErrorSequencePreset.DeviceError);
}

void hook::AppError(const char* tag, const char* msg, const char* file, int line) {
    Debug.XWriteLine("[error]: {}: {} at {}:{}", tag, msg, file, line);
    Debug.FlashErrorLightSequence(LedErrorSequencePreset.AppError);
}

void hook::AppErrorCode(const char* tag, const char* msg, int32_t code, const char* file, int line) {
    Debug.XWriteLine("[error]: {}: {} code: {} at {}:{}", tag, msg, code, file, line);
    Debug.FlashErrorLightSequence(LedErrorSequencePreset.AppError);
}

void hook::DeviceLog(const char* tag, const char* msg) {
    Debug.WriteLine("[{}]: {}", tag, msg);
}

void hook::AppLog(const char* tag, const char* msg) {
    Debug.XWriteLine("[{}]: {}", tag, msg);
}
