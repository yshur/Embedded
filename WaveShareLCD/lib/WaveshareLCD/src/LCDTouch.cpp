/*****************************************************************************
 * | File        : LCDTouch.cpp
 * | Author      : Refactored to OOP (based on Waveshare team code)
 * | Function    : XPT2046 Touch Controller Driver implementation
 *****************************************************************************/

#include "LCDTouch.h"
#include <stdlib.h>
#include <math.h>

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
#define READ_TIMES      5       // Number of ADC readings to average
#define ERR_RANGE       50      // Tolerance for double-read validation

//------------------------------------------------------------------------------
// Constructors
//------------------------------------------------------------------------------

LCDTouch::LCDTouch(WaveshareLCD& lcd)
    : _lcd(lcd), _pins(), _status(0),
      _rawX(0), _rawY(0), _rawX0(0), _rawY0(0),
      _drawPoint{0, 0, Colors::BLUE, DotPixel::PX_2X2},
      _scanDir(SCAN_DIR_DEFAULT),
      _xFac(0), _yFac(0), _xOff(0), _yOff(0) {
}

LCDTouch::LCDTouch(WaveshareLCD& lcd, const TouchPins& pins)
    : _lcd(lcd), _pins(pins), _status(0),
      _rawX(0), _rawY(0), _rawX0(0), _rawY0(0),
      _drawPoint{0, 0, Colors::BLUE, DotPixel::PX_2X2},
      _scanDir(SCAN_DIR_DEFAULT),
      _xFac(0), _yFac(0), _xOff(0), _yOff(0) {
}

//------------------------------------------------------------------------------
// Initialization
//------------------------------------------------------------------------------

void LCDTouch::begin() {
    pinMode(_pins.cs, OUTPUT);
    pinMode(_pins.irq, INPUT);
    pinMode(_pins.busy, INPUT);

    digitalWrite(_pins.irq, HIGH);
    csHigh();

    _scanDir = _lcd.getScanDir();
    loadDefaultCalibration();
}

//------------------------------------------------------------------------------
// ADC Reading
//------------------------------------------------------------------------------

void LCDTouch::readADC(POINT& x, POINT& y) {
    POINT data = 0;

    // Temporarily lower SPI clock for touch controller
    SPI.endTransaction();
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

    csLow();

    // Read X coordinate (0xD0 command)
    SPI.transfer(0xD0);
    delayMicroseconds(200);
    data = SPI.transfer(0x00);
    data <<= 8;
    data |= SPI.transfer(0x00);
    data >>= 3;
    x = data;

    // Read Y coordinate (0x90 command)
    SPI.transfer(0x90);
    delayMicroseconds(200);
    data = SPI.transfer(0x00);
    data <<= 8;
    data |= SPI.transfer(0x00);
    data >>= 3;
    y = data;

    csHigh();

    // Restore SPI clock
    SPI.endTransaction();
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
}

void LCDTouch::readADCAverage(POINT& x, POINT& y) {
    POINT xBuff[READ_TIMES], yBuff[READ_TIMES];
    POINT tempX, tempY;

    // Read multiple samples
    for (uint8_t i = 0; i < READ_TIMES; i++) {
        readADC(tempX, tempY);
        xBuff[i] = tempX;
        yBuff[i] = tempY;
        delayMicroseconds(200);
    }

    // Process X values: find and exclude min/max
    POINT minVal = xBuff[0];
    uint8_t minIdx = 0;
    for (uint8_t i = 1; i < READ_TIMES; i++) {
        if (xBuff[i] < minVal) {
            minVal = xBuff[i];
            minIdx = i;
        }
    }
    // Swap min to position 0
    POINT temp = xBuff[0];
    xBuff[0] = xBuff[minIdx];
    xBuff[minIdx] = temp;

    POINT maxVal = xBuff[1];
    uint8_t maxIdx = 1;
    for (uint8_t i = 2; i < READ_TIMES; i++) {
        if (xBuff[i] > maxVal) {
            maxVal = xBuff[i];
            maxIdx = i;
        }
    }
    // Swap max to last position
    temp = xBuff[READ_TIMES - 1];
    xBuff[READ_TIMES - 1] = xBuff[maxIdx];
    xBuff[maxIdx] = temp;

    // Sum middle values
    POINT xSum = 0;
    for (uint8_t i = 1; i < READ_TIMES - 1; i++) {
        xSum += xBuff[i];
    }
    x = xSum / (READ_TIMES - 2);

    // Process Y values: find and exclude min/max
    minVal = yBuff[0];
    minIdx = 0;
    for (uint8_t i = 1; i < READ_TIMES; i++) {
        if (yBuff[i] < minVal) {
            minVal = yBuff[i];
            minIdx = i;
        }
    }
    temp = yBuff[0];
    yBuff[0] = yBuff[minIdx];
    yBuff[minIdx] = temp;

    maxVal = yBuff[1];
    maxIdx = 1;
    for (uint8_t i = 2; i < READ_TIMES; i++) {
        if (yBuff[i] > maxVal) {
            maxVal = yBuff[i];
            maxIdx = i;
        }
    }
    temp = yBuff[READ_TIMES - 1];
    yBuff[READ_TIMES - 1] = yBuff[maxIdx];
    yBuff[maxIdx] = temp;

    POINT ySum = 0;
    for (uint8_t i = 1; i < READ_TIMES - 1; i++) {
        ySum += yBuff[i];
    }
    y = ySum / (READ_TIMES - 2);
}

bool LCDTouch::readTwiceADC(POINT& x, POINT& y) {
    POINT x1, y1, x2, y2;

    readADCAverage(x1, y1);
    readADCAverage(x2, y2);

    // Check if readings are within tolerance
    if (((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE)) &&
        ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE))) {
        x = (x1 + x2) / 2;
        y = (y1 + y2) / 2;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Touch Scanning
//------------------------------------------------------------------------------

bool LCDTouch::scan(bool calibrationMode) {
    if (!isTouching()) {
        // Not pressed
        if (_status & TP_PRESS_DOWN) {
            _status &= ~TP_PRESS_DOWN;
        } else {
            _rawX0 = 0;
            _rawY0 = 0;
            _rawX = 0xFFFF;
            _rawY = 0xFFFF;
        }
        return false;
    }

    // Touch detected - read coordinates
    if (calibrationMode) {
        readTwiceADC(_rawX, _rawY);
    } else {
        readTwiceADC(_rawX, _rawY);

        // Convert to screen coordinates based on scan direction
        const LCDInfo& info = _lcd.getInfo();

        if (_scanDir == ScanDir::R2L_D2U) {
            _drawPoint.x = _xFac * _rawX + _xOff;
            _drawPoint.y = _yFac * _rawY + _yOff;
        } else if (_scanDir == ScanDir::L2R_U2D) {
            _drawPoint.x = info.width - _xFac * _rawX - _xOff;
            _drawPoint.y = info.height - _yFac * _rawY - _yOff;
        } else if (_scanDir == ScanDir::U2D_R2L) {
            _drawPoint.x = _xFac * _rawY + _xOff;
            _drawPoint.y = _yFac * _rawX + _yOff;
        } else {
            _drawPoint.x = info.width - _xFac * _rawY - _xOff;
            _drawPoint.y = info.height - _yFac * _rawX - _yOff;
        }
    }

    // Update press state
    if ((_status & TP_PRESS_DOWN) == 0) {
        _status = TP_PRESS_DOWN | TP_PRESSED;
        _rawX0 = _rawX;
        _rawY0 = _rawY;
    }

    return true;
}

//------------------------------------------------------------------------------
// Calibration
//------------------------------------------------------------------------------

void LCDTouch::loadDefaultCalibration() {
    if (_scanDir == ScanDir::D2U_L2R) {
        _xFac = -0.132443f;
        _yFac = 0.089997f;
        _xOff = 516;
        _yOff = -22;
    } else if (_scanDir == ScanDir::L2R_U2D) {
        _xFac = 0.089697f;
        _yFac = 0.134792f;
        _xOff = -21;
        _yOff = -39;
    } else if (_scanDir == ScanDir::R2L_D2U) {
        _xFac = 0.089915f;
        _yFac = 0.133178f;
        _xOff = -22;
        _yOff = -38;
    } else if (_scanDir == ScanDir::U2D_R2L) {
        _xFac = -0.132906f;
        _yFac = 0.087964f;
        _xOff = 517;
        _yOff = -20;
    } else {
        // Unsupported direction - show error
        _lcd.clear(LCD_BACKGROUND);
        _lcd.drawString(0, 60,
            "Touch calibration not supported for this direction",
            &Font16, FONT_BACKGROUND, Colors::RED);
    }
}

void LCDTouch::setCalibration(float xFac, float yFac, int xOff, int yOff) {
    _xFac = xFac;
    _yFac = yFac;
    _xOff = xOff;
    _yOff = yOff;
}

void LCDTouch::calibrate() {
    // Simple calibration routine
    // For full calibration, see LCD_Touch_Calibrate.cpp
    loadDefaultCalibration();
}
