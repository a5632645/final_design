#include "DelayAllocator.hpp"
#include <iostream>
#include <format>

static constexpr uint32_t kNumTCMRamDelayLines = 64;
static dsp::DelayLine tcmRamDelayLines[kNumTCMRamDelayLines];
static constexpr uint32_t kNumSDRAMDelayLines = 64;
static dsp::DelayLine sdramDelayLines[kNumSDRAMDelayLines];

static constexpr uint32_t kNumDelayLines = kNumSDRAMDelayLines + kNumTCMRamDelayLines;
static dsp::DelayLine* unusedDelayLines[kNumDelayLines];
static uint32_t unusedDelayLinesTop = 0;

namespace dsp {

void DelayAllocator::Init() {
    for (int i = 0; i < kNumTCMRamDelayLines; i++) {
        unusedDelayLines[unusedDelayLinesTop++] = &tcmRamDelayLines[i];
    }
    for (int i = 0; i < kNumSDRAMDelayLines; i++) {
        unusedDelayLines[unusedDelayLinesTop++] = &sdramDelayLines[i];
    }
}

DelayLine* DelayAllocator::GetDelayLine() {
    if (unusedDelayLinesTop > 0) {
        auto* ptr = unusedDelayLines[--unusedDelayLinesTop];
        return ptr;
    }
    return nullptr;
}

void DelayAllocator::ReleaseDelayLine(DelayLine* delayLine) {
    unusedDelayLines[unusedDelayLinesTop++] = delayLine;
}

}
