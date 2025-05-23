#pragma once

namespace utli {

static constexpr auto Lerp(auto in0, auto in2, auto x) {
    return in0 + x * (in2 - in0);
}

}