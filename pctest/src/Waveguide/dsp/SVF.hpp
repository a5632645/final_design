#pragma once

namespace dsp {

class SVF {
public:
    void Init(float sampleRate);
    float Process(float in);
    float GetLowPass() const { return lowpass_; }
    float GetHighPass() const { return highpass_; }
    float GetBandPass() const { return bandpass_; }
    float GetBandReject() const { return bandreject_; }
    void SetCutoffFreq(float freq);
    void SetQ(float q) { q_ = q; }
private:
    float lowpass_{};
    float highpass_{};
    float bandpass_{};
    float bandreject_{};
    float sampleRate_{};
    float f_{};
    float q_{};
    float latch1_{};
    float latch2_{};
};

}