// ==================================================================================
// Copyright (c) 2017 HiFi-LoFi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ==================================================================================

#ifndef _AUDIOFFT_H
#define _AUDIOFFT_H

#include <cstddef>
#include <memory>
#include <array>

namespace audiofft {

namespace detail {
static constexpr bool IsPowerOf2(size_t val) {
  return (val == 1 || (val & (val-1)) == 0);
}

static constexpr size_t IntSqrt(size_t val) {
  for (size_t i = 1; i <= val; ++i) {
    if (i * i >= val) {
      return i;
    }
  }
  return 0;
}

extern void fft(float* data, float* re, float* im, size_t size, int* ip, float* w);
extern void ifft(float* data, const float* re, const float* im, size_t size, int* ip, float* w);
extern void makewt(int n, int* ip, float* w);
extern void makect(int n, int* ip, float* w);
}

template<size_t SIZE>
struct AudioFFT {
  std::array<int, 2 + detail::IntSqrt(SIZE)> _ip;
  std::array<float, SIZE / 2> _w;

  AudioFFT() {
    const int size4 = static_cast<int>(SIZE) / 4;
    detail::makewt(size4, _ip.data(), _w.data());
    detail::makect(size4, _ip.data(), _w.data() + size4);
  }

  void fft(float* buffer, float* re, float* im) {
    detail::fft(buffer, re, im, SIZE, _ip.data(), _w.data());
  }

  void ifft(float* buffer, const float* re, const float* im) {
    detail::ifft(buffer, re, im, SIZE, _ip.data(), _w.data());
  }

  static_assert(detail::IsPowerOf2(SIZE), "Size must be a power of 2");
};

} // namespace audiofft

#endif // Header guard
