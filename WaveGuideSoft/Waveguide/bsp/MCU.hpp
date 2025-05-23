#pragma once

namespace bsp {

class CMCU {
public:
    void Init();
    void MpuInitDmaMemory();
    void InitD1Memory();
    void InitD2Memory();
    void InitD3Memory();
    void InitITCRAM();
};

namespace internal {
struct InternalMCU {
    inline static CMCU instance;
};
}

inline static auto& MCU = internal::InternalMCU::instance;

}