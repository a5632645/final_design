#pragma once

namespace utli {

static constexpr auto Clamp(auto value, auto min, auto max) {
    return value < min ? min : value > max ? max : value;
}

}