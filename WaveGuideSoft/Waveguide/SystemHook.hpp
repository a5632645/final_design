#pragma once

namespace hook {

void DeviceError(const char* tag, const char* msg, const char* file, int line);
#define DEVICE_ERROR(tag, msg) hook::DeviceError(tag, msg, __FILE__, __LINE__);

void DeviceErrorCode(const char* tag, const char* msg, int32_t code, const char* file, int line);
#define DEVICE_ERROR_CODE(tag, msg, code) hook::DeviceErrorCode(tag, msg, code, __FILE__, __LINE__);

void AppError(const char* tag, const char* msg, const char* file, int line);
#define APP_ERROR(tag, msg) hook::AppError(tag, msg, __FILE__, __LINE__);

void AppErrorCode(const char* tag, const char* msg, int32_t code, const char* file, int line);
#define APP_ERROR_CODE(tag, msg, code) hook::AppErrorCode(tag, msg, code, __FILE__, __LINE__);

void DeviceLog(const char* tag, const char* msg);
#define DEVICE_LOG(tag, msg) hook::DeviceLog(tag, msg);

void AppLog(const char* tag, const char* msg);
#define APP_LOG(tag, msg) hook::AppLog(tag, msg);

}

void Error_Handler(void);
