#pragma once
#include "ParamDesc.hpp"
#include <cmath>

namespace dsp {

enum class BodyEnum {
    Banjo = 0,
    Calcani,
    Guitar,
    Klotz,
    Langhof,
    Banjo2,
    Dobro,
    Volin,
    Guzheng,
    kCount
};

extern ThreadSafeCallback gSafeCallback;
struct CSynthParams {

    struct {
        //                                              | name            |  min  |  max  |   step      |   default   | altMul
        // 1
        FloatParamDesc inhalling        { gSafeCallback, "inhaling",        -2.0f,  2.0f,       0.01f,     -0.24f,       1 };
        FloatParamDesc active           { gSafeCallback, "active",          -2.0f,  2.0f,       0.01f,       0.6f,       1 };
        FloatParamDesc blend            { gSafeCallback, "blend",            0.1f,  8.0f,        0.1f,       4.0f,       1 };
        FloatParamDesc noiseGain        { gSafeCallback, "noise amp",        0.0f,  1.0f,       0.01f,       0.3f,       1 };
        FloatParamDesc lossGain         { gSafeCallback, "loss gain",       -1.5f,  1.5f,       0.01f,     -1.05f,       1 };
        FloatParamDesc attack           { gSafeCallback, "attack",           1.0f,  1000.0f,     1.0f,       3.0f,       1 };
        FloatParamDesc release          { gSafeCallback, "release",          1.0f,  1000.0f,     1.0f,      10.0f,       1 };
        FloatParamDesc airGain          { gSafeCallback, "air amp",         -1.0f,  1.5f,       0.01f,      0.55f,       1 };
        IntParamDesc   lossLP           { gSafeCallback, "loss lp",           -24,    24,                      13,         1 };
        IntParamDesc   lossHP           { gSafeCallback, "loss hp offset",    -24,    24,                     -12,         1 };
        BoolParamDesc  lossFaster       { gSafeCallback, "loss faster",                                      false };
        // 2
        BoolParamDesc  manualPres       { gSafeCallback, "pressure conrol",                                  false };
        BoolParamDesc  autoVibrate      { gSafeCallback, "auto vibrate",                                     false };
        FloatParamDesc vibrateDepth     { gSafeCallback, "vibrate depth",    0.0f,  1.0f,       0.01f,       0.1f,       1 };
        FloatParamDesc vibrateRate      { gSafeCallback, "vibrate rate",     0.0f, 10.0f,       0.01f,      5.73f,       1 };
        FloatParamDesc vibrateAttack    { gSafeCallback, "vibrate attack",  20.0f, 5000.0f,     20.0f,    5000.0f,       1 };
        BoolParamDesc  autoTremolo      { gSafeCallback, "auto tremolo",                                     false };
        FloatParamDesc tremoloDepth     { gSafeCallback, "tremolo depth",    0.0f,  1.0f,       0.01f,       0.1f,       1 };
        FloatParamDesc tremoloRate      { gSafeCallback, "tremolo rate",     0.0f, 10.0f,       0.01f,      5.73f,       1 };
        FloatParamDesc tremoloAttack    { gSafeCallback, "tremolo attck",   20.0f, 5000.0f,     20.0f,    3000.0f,       1 };
    } reed;
    
    struct {
        //                                              | name            |  min  |  max  |   step      |   default   | altMul
        // 1
        FloatParamDesc decay            { gSafeCallback, "decay",       -6000.0f,   6000.0f,        25.0f,  1000.0f,      1 };
        BoolParamDesc  exciFaster       { gSafeCallback, "exci faster",                                       false };
        FloatParamDesc dispersion       { gSafeCallback, "dispersion",      0.0f, 1.0f/4.0f,  1.0f/400.0f,     0.0f,      1 };
        FloatParamDesc pos              { gSafeCallback, "pos",             0.0f,      0.5f,        0.01f,    0.13f,      1 };
        FloatParamDesc color            { gSafeCallback, "color",           0.0f,      1.0f,        0.01f,    0.55f,      1 };
        BoolParamDesc  lossFaster       { gSafeCallback, "loss faster",                                        true };
        IntParamDesc   lossTructionlow  { gSafeCallback, "loss in low",         0,    139,                     36,       1 };
        IntParamDesc   lossTructionHigh { gSafeCallback, "loss in high",        0,    139,                    100,       1 };
        IntParamDesc   lossOutLow       { gSafeCallback, "low out low",         0,    139,                    105,       1 };
        IntParamDesc   lossOutHigh      { gSafeCallback, "low out high",        0,    139,                    123,       1 };
        // 2
        IntParamDesc   exciTructionlow  { gSafeCallback, "exci in low",         0,    139,                     36,       1 };
        IntParamDesc   exciTructionHigh { gSafeCallback, "exci in high",        0,    139,                    100,       1 };
        IntParamDesc   exciOutLow       { gSafeCallback, "exciout low",         0,    139,                    105,       1 };
        IntParamDesc   exciOutHigh      { gSafeCallback, "exciout high",        0,    139,                    123,       1 };
    } string;
    
    struct {
        //                                              | name            |  min  |  max  |   step      |   default   | altMul
        // 1
        FloatParamDesc bowPos           { gSafeCallback, "pos",             0.01f,  0.49f,      0.01f,      0.13f,       1 };
        FloatParamDesc bowSpeed         { gSafeCallback, "speed",            0.0f,   1.0f,      0.01f,      0.08f,       1 };
        FloatParamDesc offset           { gSafeCallback, "offset",          -1.0f,   1.0f,      0.01f,      0.03f,       1 };
        FloatParamDesc slope            { gSafeCallback, "slope",            0.0f,   1.0f,      0.01f,       0.5f,       1 };
        FloatParamDesc reflectMin       { gSafeCallback, "min",              0.0f,   1.0f,      0.01f,      0.01f,       1 };
        FloatParamDesc reflectMax       { gSafeCallback, "max",              0.0f,   1.0f,      0.01f,      0.98f,       1 };
        FloatParamDesc lossGain         { gSafeCallback, "decay",           -1.0f,   1.0f,      0.01f,      0.97f,       1 };
        FloatParamDesc decay            { gSafeCallback, "decay time",       0.0f,   6000.0f,   25.0f,    2000.0f,       1 };
        BoolParamDesc  lossFaster       { gSafeCallback, "loss faster",                                      false };
        FloatParamDesc noise            { gSafeCallback, "noise",            0.0f,   0.5f,      0.01f,      0.03f,       1 };
        IntParamDesc   noiseLP          { gSafeCallback, "noise lp",            0,    139,                     80,       1 };
        // 2
        BoolParamDesc  manualSpeed      { gSafeCallback, "speed conrol",                                     false };
        BoolParamDesc  manualPitch      { gSafeCallback, "pitch conrol",                                     false };
        BoolParamDesc  autoVibrate      { gSafeCallback, "auto vibrate",                                     true };
        FloatParamDesc vibrateDepth     { gSafeCallback, "vibrate depth",    0.0f,  1.0f,       0.01f,       0.1f,       1 };
        FloatParamDesc vibrateRate      { gSafeCallback, "vibrate rate",     0.0f, 20.0f,       0.01f,      6.33f,       1 };
        FloatParamDesc vibrateAttack    { gSafeCallback, "vibrate attack",  20.0f, 5000.0f,     20.0f,    1500.0f,       1 };
        BoolParamDesc  autoTremolo      { gSafeCallback, "auto tremolo",                                     true };
        FloatParamDesc tremoloDepth     { gSafeCallback, "tremolo depth",    0.0f,  1.0f,      0.001f,      0.01f,       1 };
        FloatParamDesc tremoloRate      { gSafeCallback, "tremolo rate",     0.0f, 20.0f,       0.01f,      11.0f,       1 };
        FloatParamDesc tremoloAttack    { gSafeCallback, "tremolo attck",   20.0f, 5000.0f,     20.0f,    1500.0f,       1 };
        FloatParamDesc attack           { gSafeCallback, "attack",           1.0f,  1000.0f,     5.0f,       5.0f,       1 };
        FloatParamDesc release          { gSafeCallback, "release",          1.0f,  1000.0f,     5.0f,      60.0f,       1 };
        // 3
        IntParamDesc   lossTructionlow  { gSafeCallback, "loss in low",         0,    139,                     48,       1 };
        IntParamDesc   lossTructionHigh { gSafeCallback, "loss in high",        0,    139,                    100,       1 };
        IntParamDesc   lossOutLow       { gSafeCallback, "low out low",         0,    139,                    105,       1 };
        IntParamDesc   lossOutHigh      { gSafeCallback, "low out high",        0,    139,                    123,       1 };
    } bow;
    
    struct {
        //                       {              | name            |  min  |  max  |   step      |   default   | altMul
        FloatParamDesc rate      { gSafeCallback, "rate",           0.0f,   10.0f,     0.01f,       2.0f,        1 };
        FloatParamDesc depth     { gSafeCallback, "depth",          0.0f,   1.0f,      0.01f,       0.1f,        1 };
        FloatParamDesc drywet    { gSafeCallback, "drywet",         0.0f,   1.0f,      0.01f,       0.1f,       1 };
        FloatParamDesc decay     { gSafeCallback, "decay",          20.0f,  10000.0f,  20.0f,    1500.0f,     1 };
        FloatParamDesc size      { gSafeCallback, "size",           0.0f,   1.0f,      0.01f,       0.7f,        1 };
        IntParamDesc   lossLP    { gSafeCallback, "loss lp",           0,   139,                     130,         1 };
        IntParamDesc   interval  { gSafeCallback, "interval",         32,   256,                     128,         1 };
        FloatParamDesc earlyRefl { gSafeCallback, "early ref",      0.2f,   1.0f,      0.01f,       1.0f,        1};
        FloatParamDesc chrousRate{ gSafeCallback, "chrous smooth",0.9995f,   1.0f,   0.00002f,    0.9998f,        1};
        FloatParamDesc chrousDept{ gSafeCallback, "chrous depth",   0.0f, 256.0f,       1.0f,       64.0f,        1};
    } reverb;
    
    //                           {              | name            |  min  |  max  |   step      |   default   | altMul
    BoolParamDesc body           { gSafeCallback, "body",                                            false };
    IntParamDesc  bodyType       { gSafeCallback, "body type",         0,       8,                      0,          1 };
    FloatParamDesc wetGain       { gSafeCallback, "wet gain",     -60.0f,   60.0f,      0.1f,        0.0f,        1 };
    FloatParamDesc stretch       { gSafeCallback, "stretch",       0.25f,    3.0f,     0.01f,        1.0f,        1 };
};
extern CSynthParams SynthParams;

}
