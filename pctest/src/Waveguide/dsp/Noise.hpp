#pragma once
#include <cstdint>

namespace dsp {

class Noise {
public:
    void Init(float sampleRate) {}

    void     SetSeed(uint32_t seed);
    float    Next01();
    float    Next();
    float    Lowpassed();
    float    Lowpassed01();
    uint32_t NextUInt();
    uint32_t GetReg() const;
private:
    uint32_t reg_{};
};

} // namespace dsp
