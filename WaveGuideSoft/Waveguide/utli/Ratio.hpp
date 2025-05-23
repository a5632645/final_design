#pragma once

namespace utli {

static constexpr auto Ratio(auto min, auto max, auto value) {
    if (value < min) return 0.0f;
    else if (value > max) return 1.0f;
    else return (value - min) / (max - min);
}

}