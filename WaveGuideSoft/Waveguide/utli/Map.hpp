#pragma once

namespace utli {

static constexpr auto Map(auto toMin, auto toMax, auto fromMin, auto fromMax, auto value) {
    using ReturnType = decltype(toMin + (toMax - toMin) * (value - fromMin) / (fromMax - fromMin));

    if (value < fromMin) return static_cast<ReturnType>(toMin);
    else if (value > fromMax) return static_cast<ReturnType>(toMax);
    else return static_cast<ReturnType>(toMin + (toMax - toMin) * (value - fromMin) / (fromMax - fromMin));
}

}