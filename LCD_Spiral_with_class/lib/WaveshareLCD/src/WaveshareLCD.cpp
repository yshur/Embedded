/*****************************************************************************
 * | File        : WaveshareLCD.cpp
 * | Author      : Refactored to OOP (based on Waveshare team code)
 * | Function    : ILI9486 LCD Driver and GUI implementation
 *****************************************************************************/

#include "WaveshareLCD.h"
#include <Arduino.h>

//------------------------------------------------------------------------------
// Constructors
//------------------------------------------------------------------------------

WaveshareLCD::WaveshareLCD()
    : _pins(), _info{0}, _initialized(false) {
}

WaveshareLCD::WaveshareLCD(const LCDPins& pins)
    : _pins(pins), _info{0}, _initialized(false) {
}

//------------------------------------------------------------------------------
// Initialization
//------------------------------------------------------------------------------

void WaveshareLCD::begin(ScanDir scanDir, uint16_t backlight) {
    // Setup GPIO pins
    pinMode(_pins.cs, OUTPUT);
    pinMode(_pins.rst, OUTPUT);
    pinMode(_pins.dc, OUTPUT);

    csHigh();

    // Setup PWM for backlight if pin is configured
    if (_pins.bl > 0) {
        ledcSetup(0, 5000.0, 8);
        ledcAttachPin(_pins.bl, 0);
    }

    // Initialize SPI
    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

    // Hardware reset
    reset();

    // Set backlight
    if (backlight > 255) backlight = 255;
    if (backlight > 0) setBacklight(backlight);

    // Initialize LCD registers
    initRegisters();

    // Set scan direction
    setScanDirection(scanDir);
    delay(200);

    // Sleep out
    writeReg(0x11);
    delay(120);

    // Display on
    writeReg(0x29);

    _initialized = true;
}

void WaveshareLCD::end() {
    _initialized = false;
}

//------------------------------------------------------------------------------
// Backlight control
//------------------------------------------------------------------------------

void WaveshareLCD::setBacklight(uint16_t value) {
    if (value > 255) value = 255;
    if (_pins.bl > 0) {
        ledcWrite(0, value);
    }
}

//------------------------------------------------------------------------------
// Hardware control
//------------------------------------------------------------------------------

void WaveshareLCD::reset() {
    rstHigh();
    delay(500);
    rstLow();
    delay(500);
    rstHigh();
    delay(500);
}

void WaveshareLCD::writeReg(uint8_t reg) {
    dcCmd();
    csLow();
    SPI.transfer(reg);
    csHigh();
}

void WaveshareLCD::writeData(uint8_t data) {
    dcData();
    csLow();
    SPI.transfer(0x00);
    SPI.transfer(data);
    csHigh();
}

void WaveshareLCD::writeAllData(uint16_t data, uint32_t len) {
    dcData();
    csLow();
    for (uint32_t i = 0; i < len; i++) {
        SPI.write16(data);
    }
    csHigh();
}

void WaveshareLCD::initRegisters() {
    writeReg(0xF9);
    writeData(0x00);
    writeData(0x08);

    writeReg(0xC0);
    writeData(0x19);
    writeData(0x1a);

    writeReg(0xC1);
    writeData(0x45);
    writeData(0x00);

    writeReg(0xC2);
    writeData(0x33);

    writeReg(0xC5);
    writeData(0x00);
    writeData(0x28);

    writeReg(0xB1);
    writeData(0xA0);
    writeData(0x11);

    writeReg(0xB4);
    writeData(0x02);

    writeReg(0xB6);
    writeData(0x00);
    writeData(0x42);
    writeData(0x3B);

    writeReg(0xB7);
    writeData(0x07);

    writeReg(0xE0);
    writeData(0x1F);
    writeData(0x25);
    writeData(0x22);
    writeData(0x0B);
    writeData(0x06);
    writeData(0x0A);
    writeData(0x4E);
    writeData(0xC6);
    writeData(0x39);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);

    writeReg(0xE1);
    writeData(0x1F);
    writeData(0x3F);
    writeData(0x3F);
    writeData(0x0F);
    writeData(0x1F);
    writeData(0x0F);
    writeData(0x46);
    writeData(0x49);
    writeData(0x31);
    writeData(0x05);
    writeData(0x09);
    writeData(0x03);
    writeData(0x1C);
    writeData(0x1A);
    writeData(0x00);

    writeReg(0xF1);
    writeData(0x36);
    writeData(0x04);
    writeData(0x00);
    writeData(0x3C);
    writeData(0x0F);
    writeData(0x0F);
    writeData(0xA4);
    writeData(0x02);

    writeReg(0xF2);
    writeData(0x18);
    writeData(0xA3);
    writeData(0x12);
    writeData(0x02);
    writeData(0x32);
    writeData(0x12);
    writeData(0xFF);
    writeData(0x32);
    writeData(0x00);

    writeReg(0xF4);
    writeData(0x40);
    writeData(0x00);
    writeData(0x08);
    writeData(0x91);
    writeData(0x04);

    writeReg(0xF8);
    writeData(0x21);
    writeData(0x04);

    writeReg(0x3A);
    writeData(0x55);
}

//------------------------------------------------------------------------------
// Scan direction
//------------------------------------------------------------------------------

void WaveshareLCD::setScanDirection(ScanDir dir) {
    uint16_t memoryAccessReg = 0;
    uint16_t disFunReg = 0;

    switch (dir) {
        case ScanDir::L2R_U2D:
            memoryAccessReg = 0x08;
            disFunReg = 0x22;
            break;
        case ScanDir::L2R_D2U:
            memoryAccessReg = 0x08;
            disFunReg = 0x62;
            break;
        case ScanDir::R2L_U2D:
            memoryAccessReg = 0x08;
            disFunReg = 0x02;
            break;
        case ScanDir::R2L_D2U:
            memoryAccessReg = 0x08;
            disFunReg = 0x42;
            break;
        case ScanDir::U2D_L2R:
            memoryAccessReg = 0x28;
            disFunReg = 0x22;
            break;
        case ScanDir::U2D_R2L:
            memoryAccessReg = 0x28;
            disFunReg = 0x02;
            break;
        case ScanDir::D2U_L2R:
            memoryAccessReg = 0x28;
            disFunReg = 0x62;
            break;
        case ScanDir::D2U_R2L:
            memoryAccessReg = 0x28;
            disFunReg = 0x42;
            break;
    }

    _info.scanDir = dir;

    // Set width and height based on scan direction
    if (dir == ScanDir::L2R_U2D || dir == ScanDir::L2R_D2U ||
        dir == ScanDir::R2L_U2D || dir == ScanDir::R2L_D2U) {
        _info.width = LCD_HEIGHT;
        _info.height = LCD_WIDTH;
    } else {
        _info.width = LCD_WIDTH;
        _info.height = LCD_HEIGHT;
    }

    // Write to registers
    writeReg(0xB6);
    writeData(0x00);
    writeData(disFunReg);

    writeReg(0x36);
    writeData(memoryAccessReg);
}

//------------------------------------------------------------------------------
// Screen control
//------------------------------------------------------------------------------

void WaveshareLCD::clear(COLOR color) {
    fillArea(0, 0, _info.width, _info.height, color);
}

void WaveshareLCD::setWindow(POINT xStart, POINT yStart, POINT xEnd, POINT yEnd) {
    writeReg(0x2A);
    writeData(xStart >> 8);
    writeData(xStart & 0xFF);
    writeData((xEnd - 1) >> 8);
    writeData((xEnd - 1) & 0xFF);

    writeReg(0x2B);
    writeData(yStart >> 8);
    writeData(yStart & 0xFF);
    writeData((yEnd - 1) >> 8);
    writeData((yEnd - 1) & 0xFF);

    writeReg(0x2C);
}

void WaveshareLCD::setCursor(POINT x, POINT y) {
    setWindow(x, y, x, y);
}

void WaveshareLCD::setWindowColor(COLOR color, POINT width, POINT height) {
    writeAllData(color, (uint32_t)width * (uint32_t)height);
}

void WaveshareLCD::setPixel(POINT x, POINT y, COLOR color) {
    if (x <= _info.width && y <= _info.height) {
        setCursor(x, y);
        setWindowColor(color, 1, 1);
    }
}

void WaveshareLCD::fillArea(POINT xStart, POINT yStart, POINT xEnd, POINT yEnd, COLOR color) {
    if (xEnd > xStart && yEnd > yStart) {
        setWindow(xStart, yStart, xEnd, yEnd);
        setWindowColor(color, xEnd - xStart + 1, yEnd - yStart + 1);
    }
}

//------------------------------------------------------------------------------
// Drawing primitives
//------------------------------------------------------------------------------

void WaveshareLCD::swapPoints(POINT& p1, POINT& p2) {
    POINT temp = p1;
    p1 = p2;
    p2 = temp;
}

void WaveshareLCD::drawPoint(POINT x, POINT y, COLOR color,
                              DotPixel dotSize, DotStyle dotStyle) {
    if (x > _info.width || y > _info.height) {
        return;
    }

    int16_t xDir, yDir;
    uint8_t size = static_cast<uint8_t>(dotSize);

    if (dotStyle == DotStyle::FILL_AROUND) {
        for (xDir = 0; xDir < 2 * size - 1; xDir++) {
            for (yDir = 0; yDir < 2 * size - 1; yDir++) {
                setPixel(x + xDir - size, y + yDir - size, color);
            }
        }
    } else {
        for (xDir = 0; xDir < size; xDir++) {
            for (yDir = 0; yDir < size; yDir++) {
                setPixel(x + xDir - 1, y + yDir - 1, color);
            }
        }
    }
}

void WaveshareLCD::drawHorizontalLine(POINT xStart, POINT xEnd, POINT y,
                                       COLOR color, LineStyle style,
                                       DotPixel dotSize) {
    uint8_t size = static_cast<uint8_t>(dotSize);
    if (style == LineStyle::SOLID) {
        fillArea(xStart, y, xEnd, y + size, color);
    } else {
        POINT x;
        for (x = xStart; x <= xEnd - size; x += 2 * size) {
            fillArea(x, y, x + size, y + size, color);
        }
        if (x < xEnd) {
            fillArea(x, y, xEnd, y + size, color);
        }
    }
}

void WaveshareLCD::drawVerticalLine(POINT x, POINT yStart, POINT yEnd,
                                     COLOR color, LineStyle style,
                                     DotPixel dotSize) {
    uint8_t size = static_cast<uint8_t>(dotSize);
    if (style == LineStyle::SOLID) {
        fillArea(x, yStart, x + size, yEnd, color);
    } else {
        POINT y;
        for (y = yStart; y <= yEnd - size; y += 2 * size) {
            fillArea(x, y, x + size, y + size, color);
        }
        if (y < yEnd) {
            fillArea(x, y, x + size, yEnd, color);
        }
    }
}

void WaveshareLCD::drawLine(POINT xStart, POINT yStart, POINT xEnd, POINT yEnd,
                             COLOR color, LineStyle lineStyle, DotPixel dotSize) {
    if (xStart > _info.width || yStart > _info.height ||
        xEnd > _info.width || yEnd > _info.height) {
        return;
    }

    if (xStart > xEnd) swapPoints(xStart, xEnd);
    if (yStart > yEnd) swapPoints(yStart, yEnd);

    if (yStart == yEnd) {
        drawHorizontalLine(xStart, xEnd, yStart, color, lineStyle, dotSize);
        return;
    }

    if (xStart == xEnd) {
        drawVerticalLine(xStart, yStart, yEnd, color, lineStyle, dotSize);
        return;
    }

    // Bresenham's line algorithm for diagonal lines
    POINT xPoint = xStart;
    POINT yPoint = yStart;
    int dx = (int)xEnd - (int)xStart >= 0 ? xEnd - xStart : xStart - xEnd;
    int dy = (int)yEnd - (int)yStart <= 0 ? yEnd - yStart : yStart - yEnd;

    int xAddWay = xStart < xEnd ? 1 : -1;
    int yAddWay = yStart < yEnd ? 1 : -1;

    int esp = dx + dy;
    char lineStyleTemp = 0;

    for (;;) {
        lineStyleTemp++;
        if (lineStyle == LineStyle::DOTTED && lineStyleTemp % 3 == 0) {
            drawPoint(xPoint, yPoint, LCD_BACKGROUND, dotSize, DOT_STYLE_DEFAULT);
            lineStyleTemp = 0;
        } else {
            drawPoint(xPoint, yPoint, color, dotSize, DOT_STYLE_DEFAULT);
        }

        if (2 * esp >= dy) {
            if (xPoint == xEnd) break;
            esp += dy;
            xPoint += xAddWay;
        }
        if (2 * esp <= dx) {
            if (yPoint == yEnd) break;
            esp += dx;
            yPoint += yAddWay;
        }
    }
}

void WaveshareLCD::drawRectangle(POINT xStart, POINT yStart, POINT xEnd, POINT yEnd,
                                  COLOR color, DrawFill fill,
                                  DotPixel dotSize, LineStyle lineStyle) {
    if (xStart > _info.width || yStart > _info.height ||
        xEnd > _info.width || yEnd > _info.height) {
        return;
    }

    if (xStart > xEnd) swapPoints(xStart, xEnd);
    if (yStart > yEnd) swapPoints(yStart, yEnd);

    uint8_t size = static_cast<uint8_t>(dotSize);

    if (fill == DrawFill::FULL) {
        fillArea(xStart, yStart, xEnd, yEnd, color);
    } else {
        drawLine(xStart, yStart, xEnd + size, yStart, color, lineStyle, dotSize);
        drawLine(xStart, yStart, xStart, yEnd + size, color, lineStyle, dotSize);
        drawLine(xEnd, yEnd + size, xEnd, yStart, color, lineStyle, dotSize);
        drawLine(xEnd + size, yEnd, xStart, yEnd, color, lineStyle, dotSize);
    }
}

void WaveshareLCD::drawCircle(POINT xCenter, POINT yCenter, LENGTH radius,
                               COLOR color, DrawFill fill, DotPixel dotSize) {
    if (xCenter > _info.width || yCenter >= _info.height) {
        return;
    }

    int16_t xCurrent = 0;
    int16_t yCurrent = radius;
    int16_t esp = 3 - (radius << 1);
    int16_t sCountY;

    if (fill == DrawFill::FULL) {
        while (xCurrent <= yCurrent) {
            for (sCountY = xCurrent; sCountY <= yCurrent; sCountY++) {
                drawPoint(xCenter + xCurrent, yCenter + sCountY, color, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                drawPoint(xCenter - xCurrent, yCenter + sCountY, color, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                drawPoint(xCenter - sCountY, yCenter + xCurrent, color, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                drawPoint(xCenter - sCountY, yCenter - xCurrent, color, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                drawPoint(xCenter - xCurrent, yCenter - sCountY, color, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                drawPoint(xCenter + xCurrent, yCenter - sCountY, color, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                drawPoint(xCenter + sCountY, yCenter - xCurrent, color, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                drawPoint(xCenter + sCountY, yCenter + xCurrent, color, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
            }
            if (esp < 0) {
                esp += 4 * xCurrent + 6;
            } else {
                esp += 10 + 4 * (xCurrent - yCurrent);
                yCurrent--;
            }
            xCurrent++;
        }
    } else {
        while (xCurrent <= yCurrent) {
            drawPoint(xCenter + xCurrent, yCenter + yCurrent, color, dotSize, DOT_STYLE_DEFAULT);
            drawPoint(xCenter - xCurrent, yCenter + yCurrent, color, dotSize, DOT_STYLE_DEFAULT);
            drawPoint(xCenter - yCurrent, yCenter + xCurrent, color, dotSize, DOT_STYLE_DEFAULT);
            drawPoint(xCenter - yCurrent, yCenter - xCurrent, color, dotSize, DOT_STYLE_DEFAULT);
            drawPoint(xCenter - xCurrent, yCenter - yCurrent, color, dotSize, DOT_STYLE_DEFAULT);
            drawPoint(xCenter + xCurrent, yCenter - yCurrent, color, dotSize, DOT_STYLE_DEFAULT);
            drawPoint(xCenter + yCurrent, yCenter - xCurrent, color, dotSize, DOT_STYLE_DEFAULT);
            drawPoint(xCenter + yCurrent, yCenter + xCurrent, color, dotSize, DOT_STYLE_DEFAULT);

            if (esp < 0) {
                esp += 4 * xCurrent + 6;
            } else {
                esp += 10 + 4 * (xCurrent - yCurrent);
                yCurrent--;
            }
            xCurrent++;
        }
    }
}

//------------------------------------------------------------------------------
// Text and numbers
//------------------------------------------------------------------------------

void WaveshareLCD::drawChar(POINT x, POINT y, char ch,
                             sFONT* font, COLOR bgColor, COLOR fgColor) {
    if (x > _info.width || y > _info.height) {
        return;
    }

    uint32_t charOffset = (ch - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char* ptr = &font->table[charOffset];

    for (POINT page = 0; page < font->Height; page++) {
        for (POINT col = 0; col < font->Width; col++) {
            if (FONT_BACKGROUND == bgColor) {
                if (pgm_read_byte(ptr) & (0x80 >> (col % 8))) {
                    drawPoint(x + col, y + page, fgColor, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                }
            } else {
                if (pgm_read_byte(ptr) & (0x80 >> (col % 8))) {
                    drawPoint(x + col, y + page, fgColor, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                } else {
                    drawPoint(x + col, y + page, bgColor, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                }
            }
            if (col % 8 == 7) ptr++;
        }
        if (font->Width % 8 != 0) ptr++;
    }
}

void WaveshareLCD::drawString(POINT x, POINT y, const char* str,
                               sFONT* font, COLOR bgColor, COLOR fgColor) {
    POINT xPoint = x;
    POINT yPoint = y;

    if (x > _info.width || y > _info.height) {
        return;
    }

    while (*str != '\0') {
        if ((xPoint + font->Width) > _info.width) {
            xPoint = x;
            yPoint += font->Height;
        }
        if ((yPoint + font->Height) > _info.height) {
            xPoint = x;
            yPoint = y;
        }
        drawChar(xPoint, yPoint, *str, font, bgColor, fgColor);
        str++;
        xPoint += font->Width;
    }
}

void WaveshareLCD::drawNumber(POINT x, POINT y, int32_t number,
                               sFONT* font, COLOR bgColor, COLOR fgColor) {
    char strArray[256] = {0};
    char numArray[256] = {0};
    int16_t numBit = 0, strBit = 0;

    if (x > _info.width || y > _info.height) {
        return;
    }

    while (number) {
        numArray[numBit] = number % 10 + '0';
        numBit++;
        number /= 10;
    }

    while (numBit > 0) {
        strArray[strBit] = numArray[numBit - 1];
        strBit++;
        numBit--;
    }

    drawString(x, y, strArray, font, bgColor, fgColor);
}

//------------------------------------------------------------------------------
// Bitmap display
//------------------------------------------------------------------------------

void WaveshareLCD::drawBitmap(POINT x, POINT y, const uint8_t* bitmap,
                               POINT width, POINT height) {
    POINT i, j, byteWidth = (width + 7) / 8;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            if (*(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
                drawPoint(x + i, y + j, Colors::WHITE, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
            }
        }
    }
}

void WaveshareLCD::drawGrayMap(POINT x, POINT y, const uint8_t* graymap) {
    char gray = *(graymap + 1);
    POINT width = (*(graymap + 3) << 8) | (*(graymap + 2));
    POINT height = (*(graymap + 5) << 8) | (*(graymap + 4));

    if (gray == 0x04) {
        graymap = graymap + 6;
        for (POINT j = 0; j < height; j++) {
            for (POINT i = 0; i < width / 2; i++) {
                drawPoint(x + i * 2, y + j, ~(*graymap >> 4), DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                drawPoint(x + i * 2 + 1, y + j, ~*graymap, DOT_PIXEL_DEFAULT, DOT_STYLE_DEFAULT);
                graymap++;
            }
        }
    }
}

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

sFONT* WaveshareLCD::getFontForSize(POINT dx, POINT dy) {
    if (dx > Font24.Width && dy > Font24.Height) {
        return &Font24;
    } else if (dx > Font20.Width && dx < Font24.Width &&
               dy > Font20.Height && dy < Font24.Height) {
        return &Font20;
    } else if (dx > Font16.Width && dx < Font20.Width &&
               dy > Font16.Height && dy < Font20.Height) {
        return &Font16;
    } else if (dx > Font12.Width && dx < Font16.Width &&
               dy > Font12.Height && dy < Font16.Height) {
        return &Font12;
    } else if (dx > Font8.Width && dx < Font12.Width &&
               dy > Font8.Height && dy < Font12.Height) {
        return &Font8;
    }
    return nullptr;
}
