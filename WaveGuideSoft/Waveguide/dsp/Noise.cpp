#include "Noise.hpp"
#include <limits>

namespace dsp {

void Noise::SetSeed(uint32_t seed) {
    reg_ = seed;
}

float Noise::Next01() {
    reg_ *= 1103515245;
    reg_ += 12345;
    return reg_ / static_cast<float>(std::numeric_limits<uint32_t>::max());
}

float Noise::Next() {
    auto e = Next01();
    return e * 2 - 1;
}

float Noise::Lowpassed01() {
    float last = reg_ / static_cast<float>(std::numeric_limits<uint32_t>::max());
    float curr = Next01();
    return (last + curr) * 0.5f;
}

float Noise::Lowpassed() {   
    return Lowpassed01() * 0.5f - 0.5f;
}

uint32_t Noise::NextUInt()
{
    reg_ *= 1103515245;
    reg_ += 12345;
    return reg_;
}

uint32_t Noise::GetReg() const {
    return reg_;
}

}