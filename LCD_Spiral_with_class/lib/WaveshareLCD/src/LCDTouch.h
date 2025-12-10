/*****************************************************************************
 * | File        : LCDTouch.h
 * | Author      : Refactored to OOP (based on Waveshare team code)
 * | Function    : XPT2046 Touch Controller Driver class
 * | Info        : Object-oriented interface for touch panel
 * |
 * | Usage:
 * |   WaveshareLCD lcd;
 * |   LCDTouch touch(lcd);               // Use default pins
 * |   LCDTouch touch(lcd, TouchPins(4,16,12)); // Custom pins
 * |   lcd.begin();
 * |   touch.begin();
 * |   if (touch.scan()) {
 * |       POINT x = touch.getX();
 * |       POINT y = touch.getY();
 * |   }
 *****************************************************************************/

#ifndef __LCD_TOUCH_H
#define __LCD_TOUCH_H

#include <Arduino.h>
#include <SPI.h>
#include "LCDTypes.h"
#include "WaveshareLCD.h"

// Touch status flags
#define TP_PRESS_DOWN   0x80
#define TP_PRESSED      0x40

class LCDTouch {
public:
    //--------------------------------------------------------------------------
    // Touch point structure
    //--------------------------------------------------------------------------
    struct TouchPoint {
        POINT x;
        POINT y;
        COLOR color;
        DotPixel dotSize;
    };

    //--------------------------------------------------------------------------
    // Constructors
    //--------------------------------------------------------------------------
    explicit LCDTouch(WaveshareLCD& lcd);
    LCDTouch(WaveshareLCD& lcd, const TouchPins& pins);

    //--------------------------------------------------------------------------
    // Initialization
    //--------------------------------------------------------------------------
    void begin();

    //--------------------------------------------------------------------------
    // Touch scanning
    //--------------------------------------------------------------------------
    bool scan(bool calibrationMode = false);

    //--------------------------------------------------------------------------
    // Touch state
    //--------------------------------------------------------------------------
    bool isPressed() const { return (_status & TP_PRESS_DOWN) != 0; }
    bool wasJustPressed() const { return (_status & TP_PRESSED) != 0; }
    void clearPressedFlag() { _status &= ~TP_PRESSED; }

    //--------------------------------------------------------------------------
    // Coordinates (raw ADC values)
    //--------------------------------------------------------------------------
    POINT getRawX() const { return _rawX; }
    POINT getRawY() const { return _rawY; }
    POINT getInitialRawX() const { return _rawX0; }
    POINT getInitialRawY() const { return _rawY0; }

    //--------------------------------------------------------------------------
    // Coordinates (screen coordinates after calibration)
    //--------------------------------------------------------------------------
    POINT getX() const { return _drawPoint.x; }
    POINT getY() const { return _drawPoint.y; }
    const TouchPoint& getPoint() const { return _drawPoint; }

    //--------------------------------------------------------------------------
    // Drawing color and size for paint applications
    //--------------------------------------------------------------------------
    void setColor(COLOR color) { _drawPoint.color = color; }
    COLOR getColor() const { return _drawPoint.color; }
    void setDotSize(DotPixel size) { _drawPoint.dotSize = size; }
    DotPixel getDotSize() const { return _drawPoint.dotSize; }

    //--------------------------------------------------------------------------
    // Calibration
    //--------------------------------------------------------------------------
    void calibrate();
    void setCalibration(float xFac, float yFac, int xOff, int yOff);

    //--------------------------------------------------------------------------
    // IRQ pin check
    //--------------------------------------------------------------------------
    bool isTouching() const { return digitalRead(_pins.irq) == LOW; }

private:
    //--------------------------------------------------------------------------
    // References
    //--------------------------------------------------------------------------
    WaveshareLCD& _lcd;
    TouchPins _pins;

    //--------------------------------------------------------------------------
    // State
    //--------------------------------------------------------------------------
    uint8_t _status;
    POINT _rawX, _rawY;
    POINT _rawX0, _rawY0;
    TouchPoint _drawPoint;
    ScanDir _scanDir;

    //--------------------------------------------------------------------------
    // Calibration factors
    //--------------------------------------------------------------------------
    float _xFac;
    float _yFac;
    int _xOff;
    int _yOff;

    //--------------------------------------------------------------------------
    // Private methods
    //--------------------------------------------------------------------------
    void readADC(POINT& x, POINT& y);
    void readADCAverage(POINT& x, POINT& y);
    bool readTwiceADC(POINT& x, POINT& y);
    void loadDefaultCalibration();

    //--------------------------------------------------------------------------
    // GPIO control
    //--------------------------------------------------------------------------
    inline void csLow() { digitalWrite(_pins.cs, LOW); }
    inline void csHigh() { digitalWrite(_pins.cs, HIGH); }
};

#endif // __LCD_TOUCH_H
