/*****************************************************************************
 * | File        : WaveshareLCD.h
 * | Author      : Refactored to OOP (based on Waveshare team code)
 * | Function    : ILI9486 LCD Driver and GUI class
 * | Info        : Object-oriented interface for Waveshare 3.5" LCD
 * |
 * | Usage:
 * |   WaveshareLCD lcd;                    // Use default pins
 * |   WaveshareLCD lcd(LCDPins(15,26,17,0)); // Custom pins
 * |   lcd.begin();                         // Initialize
 * |   lcd.clear(Colors::WHITE);            // Clear screen
 * |   lcd.drawLine(0, 0, 100, 100, Colors::RED);
 * |   lcd.drawString(10, 10, "Hello", &Font24, Colors::WHITE, Colors::BLUE);
 *****************************************************************************/

#ifndef __WAVESHARE_LCD_H
#define __WAVESHARE_LCD_H

#include <Arduino.h>
#include <SPI.h>
#include "LCDTypes.h"
#include "fonts/fonts.h"

class WaveshareLCD {
public:
    //--------------------------------------------------------------------------
    // Constructors
    //--------------------------------------------------------------------------
    WaveshareLCD();
    explicit WaveshareLCD(const LCDPins& pins);

    //--------------------------------------------------------------------------
    // Initialization
    //--------------------------------------------------------------------------
    void begin(ScanDir scanDir = SCAN_DIR_DEFAULT, uint16_t backlight = 200);
    void end();

    //--------------------------------------------------------------------------
    // Display properties
    //--------------------------------------------------------------------------
    LENGTH getWidth() const { return _info.width; }
    LENGTH getHeight() const { return _info.height; }
    ScanDir getScanDir() const { return _info.scanDir; }
    const LCDInfo& getInfo() const { return _info; }

    //--------------------------------------------------------------------------
    // Backlight control
    //--------------------------------------------------------------------------
    void setBacklight(uint16_t value);

    //--------------------------------------------------------------------------
    // Screen control
    //--------------------------------------------------------------------------
    void clear(COLOR color = LCD_BACKGROUND);
    void setScanDirection(ScanDir dir);

    //--------------------------------------------------------------------------
    // Low-level drawing
    //--------------------------------------------------------------------------
    void setWindow(POINT xStart, POINT yStart, POINT xEnd, POINT yEnd);
    void setCursor(POINT x, POINT y);
    void setPixel(POINT x, POINT y, COLOR color);
    void fillArea(POINT xStart, POINT yStart, POINT xEnd, POINT yEnd, COLOR color);

    //--------------------------------------------------------------------------
    // Drawing primitives
    //--------------------------------------------------------------------------
    void drawPoint(POINT x, POINT y, COLOR color,
                   DotPixel dotSize = DOT_PIXEL_DEFAULT,
                   DotStyle dotStyle = DOT_STYLE_DEFAULT);

    void drawLine(POINT xStart, POINT yStart, POINT xEnd, POINT yEnd,
                  COLOR color,
                  LineStyle lineStyle = LineStyle::SOLID,
                  DotPixel dotSize = DOT_PIXEL_DEFAULT);

    void drawRectangle(POINT xStart, POINT yStart, POINT xEnd, POINT yEnd,
                       COLOR color,
                       DrawFill fill = DrawFill::EMPTY,
                       DotPixel dotSize = DOT_PIXEL_DEFAULT,
                       LineStyle lineStyle = LineStyle::SOLID);

    void drawCircle(POINT xCenter, POINT yCenter, LENGTH radius,
                    COLOR color,
                    DrawFill fill = DrawFill::EMPTY,
                    DotPixel dotSize = DOT_PIXEL_DEFAULT);

    //--------------------------------------------------------------------------
    // Text and numbers
    //--------------------------------------------------------------------------
    void drawChar(POINT x, POINT y, char ch,
                  sFONT* font,
                  COLOR bgColor, COLOR fgColor);

    void drawString(POINT x, POINT y, const char* str,
                    sFONT* font,
                    COLOR bgColor, COLOR fgColor);

    void drawNumber(POINT x, POINT y, int32_t number,
                    sFONT* font,
                    COLOR bgColor, COLOR fgColor);

    //--------------------------------------------------------------------------
    // Bitmap display
    //--------------------------------------------------------------------------
    void drawBitmap(POINT x, POINT y, const uint8_t* bitmap,
                    POINT width, POINT height);

    void drawGrayMap(POINT x, POINT y, const uint8_t* graymap);

    //--------------------------------------------------------------------------
    // Utility functions
    //--------------------------------------------------------------------------
    sFONT* getFontForSize(POINT dx, POINT dy);

    //--------------------------------------------------------------------------
    // Low-level register access (for advanced use)
    //--------------------------------------------------------------------------
    void writeReg(uint8_t reg);
    void writeData(uint8_t data);

private:
    //--------------------------------------------------------------------------
    // Pin configuration
    //--------------------------------------------------------------------------
    LCDPins _pins;
    LCDInfo _info;
    bool _initialized;

    //--------------------------------------------------------------------------
    // Hardware control
    //--------------------------------------------------------------------------
    void reset();
    void initRegisters();
    void writeAllData(uint16_t data, uint32_t len);
    void setWindowColor(COLOR color, POINT width, POINT height);

    //--------------------------------------------------------------------------
    // Helper functions
    //--------------------------------------------------------------------------
    void drawHorizontalLine(POINT xStart, POINT xEnd, POINT y,
                            COLOR color, LineStyle style, DotPixel dotSize);
    void drawVerticalLine(POINT x, POINT yStart, POINT yEnd,
                          COLOR color, LineStyle style, DotPixel dotSize);
    static void swapPoints(POINT& p1, POINT& p2);

    //--------------------------------------------------------------------------
    // GPIO control macros as inline functions
    //--------------------------------------------------------------------------
    inline void csLow() { digitalWrite(_pins.cs, LOW); }
    inline void csHigh() { digitalWrite(_pins.cs, HIGH); }
    inline void rstLow() { digitalWrite(_pins.rst, LOW); }
    inline void rstHigh() { digitalWrite(_pins.rst, HIGH); }
    inline void dcCmd() { digitalWrite(_pins.dc, LOW); }
    inline void dcData() { digitalWrite(_pins.dc, HIGH); }
};

#endif // __WAVESHARE_LCD_H
