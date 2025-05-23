#include "DelayAllocator.hpp"
#include "MemAttributes.hpp"

static constexpr uint32_t kNumSDRAMDelayLines = 16;
MEM_BSS_SRAMD1 static dsp::DelayLine sdramDelayLines[kNumSDRAMDelayLines];

static constexpr uint32_t kNumDelayLines = kNumSDRAMDelayLines;
static dsp::DelayLine* unusedDelayLines[kNumDelayLines];
static uint32_t unusedDelayLinesTop = 0;

namespace dsp {

void DelayAllocator::Init() {
    for (uint32_t i = 0; i < kNumSDRAMDelayLines; i++) {
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
