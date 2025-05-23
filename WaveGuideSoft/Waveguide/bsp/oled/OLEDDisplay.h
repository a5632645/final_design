#pragma once

#define _OLED_MONO 0
#define _OLED_RGB  1
#define _OLED_TYPE _OLED_RGB

#if _OLED_TYPE == _OLED_MONO
#include "OLEDDisplayMono.h"
#else
#include "OLEDDisplayRGB.h"
#endif
