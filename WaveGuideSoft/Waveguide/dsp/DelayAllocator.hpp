#pragma once
#include "DelayLine.hpp"

namespace dsp {

struct DelayAllocator {
    static void Init();
    static DelayLine* GetDelayLine();
    static void ReleaseDelayLine(DelayLine* delayLine);
};

}