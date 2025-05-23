/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
 * Copyright (c) 2019 by Helmut Tschemernjak - www.radioshuttle.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ThingPulse invests considerable time and money to develop these open source libraries.
 * Please support us by buying our products (and not the clones) from
 * https://thingpulse.com
 *
 */

#pragma once

#include <cstdint>
#include <string_view>
#include "Rectange.hpp"
#include "usf/usf.hpp"

enum class OledColorEnum
{
    kOledBLACK = 0,
    kOledWHITE = 1,
    kOledINVERSE = 2
};

enum class OledDisplayTextAlignEnum
{
    kLeft = 0,
    kRight = 1,
    kXCenter = 2,
    kCenter = 3
};

class OLEDDisplay
{
public:
    static constexpr auto kWidth = 240;
    static constexpr auto kHeight = 96;
    static constexpr auto kBufferSize = kWidth * kHeight / 8;
    // static constexpr auto kTransferBufferSize = kBufferSize + 1;
    static constexpr auto kMaxPrintfString = 128;
    inline static char printBuffer[kMaxPrintfString];

    OLEDDisplay();

    void setColor(OledColorEnum color);
    OledColorEnum getColor();
    void Fill(OledColorEnum color);
    void setPixel(int16_t x, int16_t y);
    void setPixelColor(int16_t x, int16_t y, OledColorEnum color);
    void clearPixel(int16_t x, int16_t y);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    void drawRect(int16_t x, int16_t y, int16_t width, int16_t height);
    void fillRect(int16_t x, int16_t y, int16_t width, int16_t height);
    void drawCircle(int16_t x, int16_t y, int16_t radius);
    void drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads);
    void fillCircle(int16_t x, int16_t y, int16_t radius);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void drawHorizontalLine(int16_t x, int16_t y, int16_t length);
    void drawVerticalLine(int16_t x, int16_t y, int16_t length);
    void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);
    void drawFastImage(int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t *image);
    void drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t *xbm);
    void drawIco16x16(int16_t x, int16_t y, const uint8_t *ico, bool inverse = false);

    // ---------------------------------------- Text function ----------------------------------------
    uint16_t drawString(int16_t x, int16_t y, std::string_view text);
    uint16_t drawStringMaxWidth(int16_t x, int16_t y, uint16_t maxLineWidth, std::string_view text);
    uint16_t getStringWidth(std::string_view text);
    void setTextAlignment(OledDisplayTextAlignEnum textAlignment);
    OledDisplayTextAlignEnum GetTextAlignment() const { return textAlignment; }
    template <typename... Args> USF_CPP14_CONSTEXPR
    uint16_t FormatString(int16_t x, int16_t y, usf::StringView fmt, Args&&... args) {
        usf::StringSpan span = usf::format_to(usf::StringSpan(printBuffer, kMaxPrintfString), fmt, std::forward<Args>(args)...);
        return drawString(x, y, std::string_view(span.data(), span.size()));
    }
    template<class... Args> USF_CPP14_CONSTEXPR
    usf::StringSpan FormantToInternalBuffer(usf::StringView fmt, Args&&... args) {
        return usf::format_to(usf::StringSpan(printBuffer, kMaxPrintfString), fmt, std::forward<Args>(args)...);
    }

    // ---------------------------------------- Screen ----------------------------------------
    constexpr uint16_t getWidth(void) { return kWidth; }
    constexpr uint16_t getHeight(void) { return kHeight; }
    constexpr Rectange GetDrawAera(void) { return Rectange(0, 0, kWidth, kHeight); }
    int16_t GetFontHeight(void);

    // get transfer buffer
    uint8_t* getTransferBuffer(void) { return transfterBuffer_; }
    void SetTransferBuffer(uint8_t* buffer) { transfterBuffer_ = buffer; }

    // get display buffer
    uint8_t* getDisplayBuffer(void) { return buffer_; }
    void SetDisplayBuffer(uint8_t* buffer) { buffer_ = buffer; }

protected:
    void drawInternal(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const uint8_t* data, uint16_t offset, uint16_t bytesInData);
    uint16_t drawStringInternal(int16_t xMove, int16_t yMove, std::string_view text, uint16_t textWidth, uint16_t boundWidth);

    OledDisplayTextAlignEnum textAlignment;
    OledColorEnum color;

    uint8_t* buffer_{};
    uint8_t* transfterBuffer_{};

};
