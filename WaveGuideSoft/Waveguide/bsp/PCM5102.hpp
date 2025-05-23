#pragma once
#include <span>
#include <cstdint>
#include "Types.hpp"

namespace bsp {

class CPCM5102 {
public:
    static constexpr uint32_t kSampleRate = 48000;
    static constexpr uint32_t kBufferSize = 480 * 2;
    static constexpr uint32_t kBlockSize = CPCM5102::kBufferSize / 2;

    void Init();
    void Start();
    void Stop();
    void DeInit();

    uint32_t GetSampleRate() const { return kSampleRate; }
    uint32_t GetBufferSize() const { return kBufferSize; }
    uint32_t GetBlockSize() const { return kBlockSize; }

    std::span<StereoSample> GetNextBlock();
};

namespace internal {
struct InternalPCM5102 {
    inline static CPCM5102 instance;
};
}

inline static auto& PCM5102 = internal::InternalPCM5102::instance;

}
