#include "Reverb.hpp"
#include <numbers>
#include <cmath>
#include <cassert>

static constexpr std::array kPrimeTable {
    521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1177, 1181, 1187, 1193, 1201, 1213, 1217, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039
};

namespace dsp {
void Reverb::Init(uint32_t sampleRate) {
    sampleRate_ = sampleRate;
    noise_.Init(sampleRate);
    combAllpass1_.SetAlpha(0.4f);
    combAllpass2_.SetAlpha(0.4f);
    combAllpass3_.SetAlpha(0.4f);
    combAllpass4_.SetAlpha(0.4f);
    combAllpass5_.SetAlpha(0.4f);
    combAllpass6_.SetAlpha(0.4f);
    combAllpass7_.SetAlpha(0.4f);
    combAllpass8_.SetAlpha(0.4f);
    velvetInterval_ = 128;
    lowpass1_.Init(sampleRate);
    lowpass2_.Init(sampleRate);
    lowpass3_.Init(sampleRate);
    lowpass4_.Init(sampleRate);
    lowpass5_.Init(sampleRate);
    lowpass6_.Init(sampleRate);
    lowpass7_.Init(sampleRate);
    lowpass8_.Init(sampleRate);
}

void Reverb::NewVelvetNoise(uint32_t interval) {
    velvetInterval_ = interval;

    // velvet 1
    {
        numDelay1NegTaps_ = 0;
        numDelay1PosTaps_ = 0;
        numDelay2NegTaps_ = 0;
        numDelay2PosTaps_ = 0;
    
        uint32_t i = 0;
        gain1_ = 0;
        while (i < kFIRSize) {
            float delta = noise_.Next01() * interval;
            int32_t ddelta = static_cast<int32_t>(delta);
            int32_t pos = i + ddelta;
            if (pos < kFIRSize) {
                if (noise_.Next() < 0.0f) {
                    delay1NegTaps_[numDelay1NegTaps_++] = pos;
                }
                else {
                    delay1PosTaps_[numDelay1PosTaps_++] = pos;
                }
            }
            gain1_ += 1.0f;
            i += interval;
        }
        
        i = 0;
        gain2_ = 0;
        while (i < kFIRSize) {
            float delta = noise_.Next01() * interval;
            int32_t ddelta = static_cast<int32_t>(delta);
            int32_t pos = i + ddelta;
            if (pos < kFIRSize) {
                if (noise_.Next() < 0.0f) {
                    delay2PosTaps_[numDelay2PosTaps_++] = pos;
                }
                else {
                    delay2NegTaps_[numDelay2NegTaps_++] = pos;
                }
            }
            gain2_ += 1.0f;
            i += interval;
        }
    
        if (numDelay1NegTaps_ + numDelay1PosTaps_ == 0) {
            numDelay1PosTaps_ = 1;
            delay1PosTaps_[0] = 1;
            gain1_ = 1.0f;
        }
        if (numDelay2NegTaps_ + numDelay2PosTaps_ == 0) {
            numDelay2PosTaps_ = 1;
            delay2PosTaps_[0] = 1;
            gain2_ = 1.0f;
        }
        gain1_ = 1.0f / std::sqrt(static_cast<float>(gain1_));
        gain2_ = 1.0f / std::sqrt(static_cast<float>(gain2_));
    }
    
    // velvet 2
    {
        numDelay1NegTaps2_ = 0;
        numDelay1PosTaps2_ = 0;
        numDelay2NegTaps2_ = 0;
        numDelay2PosTaps2_ = 0;
    
        uint32_t i = 0;
        gain12_ = 0;
        while (i < kFIRSize) {
            float delta = noise_.Next01() * interval;
            int32_t ddelta = static_cast<int32_t>(delta);
            int32_t pos = i + ddelta;
            if (pos < kFIRSize) {
                if (noise_.Next() < 0.0f) {
                    delay1NegTaps2_[numDelay1NegTaps2_++] = pos;
                }
                else {
                    delay1PosTaps2_[numDelay1PosTaps2_++] = pos;
                }
            }
            gain12_ += 1.0f;
            i += interval;
        }
        
        i = 0;
        gain22_ = 0;
        while (i < kFIRSize) {
            float delta = noise_.Next01() * interval;
            int32_t ddelta = static_cast<int32_t>(delta);
            int32_t pos = i + ddelta;
            if (pos < kFIRSize) {
                if (noise_.Next() < 0.0f) {
                    delay2PosTaps2_[numDelay2PosTaps2_++] = pos;
                }
                else {
                    delay2NegTaps2_[numDelay2NegTaps2_++] = pos;
                }
            }
            gain22_ += 1.0f;
            i += interval;
        }
    
        if (numDelay1NegTaps2_ + numDelay1PosTaps2_ == 0) {
            numDelay1PosTaps2_ = 1;
            delay1PosTaps2_[0] = 1;
            gain12_ = 1.0f;
        }
        if (numDelay2NegTaps2_ + numDelay2PosTaps2_ == 0) {
            numDelay2PosTaps2_ = 1;
            delay2PosTaps2_[0] = 1;
            gain22_ = 1.0f;
        }
        gain12_ = 1.0f / std::sqrt(static_cast<float>(gain12_));
        gain22_ = 1.0f / std::sqrt(static_cast<float>(gain22_));
    }
}

void Reverb::SetEarlyReflectionSize(float size) {
    auto iSize = static_cast<uint32_t>(size * kFIRSize);
    if (iSize > kFIRSize) iSize = kFIRSize;
    if (earlyReflectionSize_ != iSize) {
        earlyReflectionSize_ = iSize;
        NewVelvetNoise(velvetInterval_);
    }
}

void Reverb::SetSize(float size) {
    auto fminIdx = size * (kPrimeTable.size() - 7);
    auto iMinIdx = static_cast<uint32_t>(fminIdx);
    auto iMaxIdx = kPrimeTable.size() - 7;
    delay1Len_ = kPrimeTable[static_cast<uint32_t>(std::lerp(iMinIdx, iMaxIdx, noise_.Next01()))];
    delay2Len_ = kPrimeTable[static_cast<uint32_t>(std::lerp(iMinIdx, iMaxIdx, noise_.Next01())) + 1];
    delay3Len_ = kPrimeTable[static_cast<uint32_t>(std::lerp(iMinIdx, iMaxIdx, noise_.Next01())) + 2];
    delay4Len_ = kPrimeTable[static_cast<uint32_t>(std::lerp(iMinIdx, iMaxIdx, noise_.Next01())) + 3];
    delay5Len_ = kPrimeTable[static_cast<uint32_t>(std::lerp(iMinIdx, iMaxIdx, noise_.Next01())) + 4];
    delay6Len_ = kPrimeTable[static_cast<uint32_t>(std::lerp(iMinIdx, iMaxIdx, noise_.Next01())) + 5];
    delay7Len_ = kPrimeTable[static_cast<uint32_t>(std::lerp(iMinIdx, iMaxIdx, noise_.Next01())) + 6];
    delay8Len_ = kPrimeTable[static_cast<uint32_t>(std::lerp(iMinIdx, iMaxIdx, noise_.Next01())) + 7];
    SetDecayTime(decayMs_);
}

void Reverb::SetModulationDepth(float depth) {
    depth_ = depth;
}

void Reverb::SetDryWet(float drywet)
{
    dryWet_ = drywet;
}

void Reverb::SetChrousRate(float rate) {
    latchAlpha_ = rate;
}

void Reverb::Process(std::span<float> buffer, std::span<float> auxBuffer) {
    float noise1 = noise_.Lowpassed01();
    float noise2 = noise_.Lowpassed01();
    float noise3 = noise_.Lowpassed01();
    float noise4 = noise_.Lowpassed01();
    float noise5 = noise_.Lowpassed01();
    float noise6 = noise_.Lowpassed01();
    float noise7 = noise_.Lowpassed01();
    float noise8 = noise_.Lowpassed01();
    for (uint32_t i = 0; i < buffer.size(); ++i) {
        // velvet fir, generate lost of impluse 
        float earlyReflectionsOut1 = 0.0f;
        float earlyReflectionsOut2 = 0.0f;
        {
            auto delay1In = buffer[i];
            delay1_[delay1WritePos_] = delay1In;
            for (uint32_t i = 0; i < numDelay1NegTaps_; ++i) {
                uint32_t idx = (delay1WritePos_ - delay1NegTaps_[i]) & kDelayMask;
                earlyReflectionsOut1 -= delay1_[idx];
            }
            for (uint32_t i = 0; i < numDelay1PosTaps_; ++i) {
                uint32_t idx = (delay1WritePos_ - delay1PosTaps_[i]) & kDelayMask;
                earlyReflectionsOut1 += delay1_[idx];
            }
            for (uint32_t i = 0; i < numDelay2NegTaps_; ++i) {
                uint32_t idx = (delay1WritePos_ - delay2NegTaps_[i]) & kDelayMask;
                earlyReflectionsOut2 -= delay1_[idx];
            }
            for (uint32_t i = 0; i < numDelay2PosTaps_; ++i) {
                uint32_t idx = (delay1WritePos_ - delay2PosTaps_[i]) & kDelayMask;
                earlyReflectionsOut2 += delay1_[idx];
            }
            ++delay1WritePos_;
            delay1WritePos_ &= kDelayMask;

            earlyReflectionsOut1 *= gain1_;
            earlyReflectionsOut2 *= gain2_;
        }
        {
            auto w = quadOscU_ - k1_ * quadOscV_;
            quadOscV_ = quadOscV_ + k2_ * w;
            quadOscU_ = w - k1_ * quadOscV_;
        }
        // fdn
        earlyReflectionsOut1 *= (quadOscU_ * depth_ + 1.0f);
        earlyReflectionsOut2 *= (quadOscV_ * depth_ + 1.0f);
        // velvet 2
        {
            delay2_[delay1WritePos2_] = earlyReflectionsOut1;
            earlyReflectionsOut1 = 0;
            for (uint32_t i = 0; i < numDelay1NegTaps2_; ++i) {
                uint32_t idx = (delay1WritePos2_ - delay1NegTaps2_[i]) & kDelayMask;
                earlyReflectionsOut1 -= delay2_[idx];
            }
            for (uint32_t i = 0; i < numDelay1PosTaps2_; ++i) {
                uint32_t idx = (delay1WritePos2_ - delay1PosTaps2_[i]) & kDelayMask;
                earlyReflectionsOut1 += delay2_[idx];
            }
            ++delay1WritePos2_;
            delay1WritePos2_ &= kDelayMask;
            earlyReflectionsOut1 *= gain12_;

            delay3_[delay1WritePos3_] = earlyReflectionsOut2;
            earlyReflectionsOut2 = 0;
            for (uint32_t i = 0; i < numDelay2NegTaps2_; ++i) {
                uint32_t idx = (delay1WritePos3_ - delay2NegTaps2_[i]) & kDelayMask;
                earlyReflectionsOut2 -= delay3_[idx];
            }
            for (uint32_t i = 0; i < numDelay2PosTaps2_; ++i) {
                uint32_t idx = (delay1WritePos3_ - delay2PosTaps2_[i]) & kDelayMask;
                earlyReflectionsOut2 += delay3_[idx];
            }
            ++delay1WritePos3_;
            delay1WritePos3_ &= kDelayMask;
            earlyReflectionsOut2 *= gain22_;
        }
        // input
        auto in1 = earlyReflectionsOut1;
        auto in2 = earlyReflectionsOut2;
        auto line1 = in1 + combAllpass1_.GetLast() * decay1_;
        auto line2 = in1 - combAllpass2_.GetLast() * decay2_;
        auto line3 = in2 + combAllpass3_.GetLast() * decay3_;
        auto line4 = in2 - combAllpass4_.GetLast() * decay4_;
        auto line5 = in1 + combAllpass5_.GetLast() * decay5_;
        auto line6 = in1 - combAllpass6_.GetLast() * decay6_;
        auto line7 = in2 + combAllpass7_.GetLast() * decay7_;
        auto line8 = in2 - combAllpass8_.GetLast() * decay8_;
        // hamonad mix
        constexpr auto mul = 0.5f / std::numbers::sqrt2_v<float>;
        auto mix1 = (line1 + line2 + line3 + line4 + line5 + line6 + line7 + line8) * mul;
        auto mix2 = (-line1 + line2 - line3 + line4 - line5 + line6 - line7 + line8) * mul;
        auto mix3 = (-line1 - line2 + line3 + line4 - line5 - line6 + line7 + line8) * mul;
        auto mix4 = (line1 - line2 - line3 + line4 + line5 - line6 - line7 + line8) * mul;
        auto mix5 = (-line1 - line2 - line3 - line4 + line5 + line6 + line7 + line8) * mul;
        auto mix6 = (line1 - line2 + line3 - line4 - line5 + line6 - line7 + line8) * mul;
        auto mix7 = (line1 + line2 - line3 - line4 - line5 - line6 + line7 + line8) * mul;
        auto mix8 = (-line1 + line2 + line3 - line4 + line5 - line6 - line7 + line8) * mul;

        // comb chrous
        {
            chrousLatch1_ = chrousLatch1_ * latchAlpha_ + noise1 * (1.0f - latchAlpha_);
            chrousLatch2_ = chrousLatch2_ * latchAlpha_ + noise2 * (1.0f - latchAlpha_);
            chrousLatch3_ = chrousLatch3_ * latchAlpha_ + noise3 * (1.0f - latchAlpha_);
            chrousLatch4_ = chrousLatch4_ * latchAlpha_ + noise4 * (1.0f - latchAlpha_);
            chrousLatch5_ = chrousLatch5_ * latchAlpha_ + noise5 * (1.0f - latchAlpha_);
            chrousLatch6_ = chrousLatch6_ * latchAlpha_ + noise6 * (1.0f - latchAlpha_);
            chrousLatch7_ = chrousLatch7_ * latchAlpha_ + noise7 * (1.0f - latchAlpha_);
            chrousLatch8_ = chrousLatch8_ * latchAlpha_ + noise8 * (1.0f - latchAlpha_);
            combAllpass1_.SetDelay(delay1Len_ - chrousLatch1_ * chrousDepth_);
            combAllpass2_.SetDelay(delay2Len_ - chrousLatch2_ * chrousDepth_);
            combAllpass3_.SetDelay(delay3Len_ - chrousLatch3_ * chrousDepth_);
            combAllpass4_.SetDelay(delay4Len_ - chrousLatch4_ * chrousDepth_);
            combAllpass5_.SetDelay(delay5Len_ - chrousLatch5_ * chrousDepth_);
            combAllpass6_.SetDelay(delay6Len_ - chrousLatch6_ * chrousDepth_);
            combAllpass7_.SetDelay(delay7Len_ - chrousLatch7_ * chrousDepth_);
            combAllpass8_.SetDelay(delay8Len_ - chrousLatch8_ * chrousDepth_);
        }

        // comb
        mix1 = lowpass1_.Process(mix1);
        mix1 = combAllpass1_.Process(mix1);
        mix2 = lowpass2_.Process(mix2);
        mix2 = combAllpass2_.Process(mix2);
        mix3 = lowpass3_.Process(mix3);
        mix3 = combAllpass3_.Process(mix3);
        mix4 = lowpass4_.Process(mix4);
        mix4 = combAllpass4_.Process(mix4);
        mix5 = lowpass5_.Process(mix5);
        mix5 = combAllpass5_.Process(mix5);
        mix6 = lowpass6_.Process(mix6);
        mix6 = combAllpass6_.Process(mix6);
        mix7 = lowpass7_.Process(mix7);
        mix7 = combAllpass7_.Process(mix7);
        mix8 = lowpass8_.Process(mix8);
        mix8 = combAllpass8_.Process(mix8);

        // output
        // auto leftOut = (line1 + line2 + line5 + line6) / 2;
        // auto rightOut = (line3 + line4 + line7 + line8) / 2;
        auto leftOut = line1;
        auto rightOut = line3;
        auto s = buffer[i];
        buffer[i] = std::lerp(s, leftOut, dryWet_);
        auxBuffer[i] = std::lerp(s, rightOut, dryWet_);
    }
}

void Reverb::SetQuadOscRate(float freq) {
    auto theta = freq / sampleRate_ * std::numbers::pi_v<float> * 2.0f;
    k1_ = std::tan(theta / 2.0f);
    k2_ = 2 * k1_ / (1 + k1_ * k1_);
}

void Reverb::SetLowpassFreq(float freq) {
    lowpass1_.SetCutoffLPF(freq);
    lowpass2_.CopyCoeff(lowpass1_);
    lowpass3_.CopyCoeff(lowpass1_);
    lowpass4_.CopyCoeff(lowpass1_);
    lowpass5_.CopyCoeff(lowpass1_);
    lowpass6_.CopyCoeff(lowpass1_);
    lowpass7_.CopyCoeff(lowpass1_);
    lowpass8_.CopyCoeff(lowpass1_);
}

void Reverb::SetDecayTime(float ms) {
    decayMs_ = ms;
    auto mul = 1.0f / (static_cast<float>(sampleRate_) * ms / 1000.0f);
    decay1_ = std::pow(10.0f, -(static_cast<float>(delay1Len_ * mul)));
    decay2_ = std::pow(10.0f, -(static_cast<float>(delay2Len_ * mul)));
    decay3_ = std::pow(10.0f, -(static_cast<float>(delay3Len_ * mul)));
    decay4_ = std::pow(10.0f, -(static_cast<float>(delay4Len_ * mul)));
    decay5_ = std::pow(10.0f, -(static_cast<float>(delay5Len_ * mul)));
    decay6_ = std::pow(10.0f, -(static_cast<float>(delay6Len_ * mul)));
    decay7_ = std::pow(10.0f, -(static_cast<float>(delay7Len_ * mul)));
    decay8_ = std::pow(10.0f, -(static_cast<float>(delay8Len_ * mul)));
}

}