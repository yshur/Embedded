#include "WaveShare.h"
#include <SPI.h>

// Touch controller chip select pin
static constexpr uint8_t TOUCH_CS_PIN = 4;

WaveShare::WaveShare()
    : _lcd()
    , _touch(_lcd)
{
}

void WaveShare::begin() {
    _lcd.begin();
    _touch.begin();

    // Apply touch calibration
    _touch.setCalibration(TOUCH_X_FAC, TOUCH_Y_FAC, TOUCH_X_OFF, TOUCH_Y_OFF);
}

uint16_t WaveShare::getWidth() const {
    return _lcd.getWidth();
}

uint16_t WaveShare::getHeight() const {
    return _lcd.getHeight();
}

void WaveShare::fillScreen(uint16_t color) {
    _lcd.clear(color);
}

void WaveShare::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    _lcd.drawRectangle(x, y, x + w - 1, y + h - 1, color, DrawFill::FULL);
}

void WaveShare::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    _lcd.drawRectangle(x, y, x + w - 1, y + h - 1, color, DrawFill::EMPTY);
}

void WaveShare::drawText(int16_t x, int16_t y, const char* text, sFONT* font,
                          uint16_t bgColor, uint16_t fgColor) {
    _lcd.drawString(x, y, text, font, bgColor, fgColor);
}

void WaveShare::drawTextCentered(int16_t x, int16_t y, int16_t w, int16_t h,
                                  const char* text, sFONT* font,
                                  uint16_t bgColor, uint16_t fgColor) {
    // Calculate text dimensions
    size_t len = strlen(text);
    int16_t textWidth = len * font->Width;
    int16_t textHeight = font->Height;

    // Center position
    int16_t textX = x + (w - textWidth) / 2;
    int16_t textY = y + (h - textHeight) / 2;

    _lcd.drawString(textX, textY, text, font, bgColor, fgColor);
}

void WaveShare::drawTextRightAligned(int16_t x, int16_t y, int16_t w,
                                      const char* text, sFONT* font,
                                      uint16_t bgColor, uint16_t fgColor) {
    // Calculate text width
    size_t len = strlen(text);
    int16_t textWidth = len * font->Width;

    // Right-align position
    int16_t textX = x + w - textWidth;

    _lcd.drawString(textX, y, text, font, bgColor, fgColor);
}

bool WaveShare::readTouch(int16_t& tx, int16_t& ty) {
    // Direct SPI read from touch controller (more reliable than IRQ-based)
    uint16_t rawX = 0, rawY = 0;

    SPI.endTransaction();
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

    digitalWrite(TOUCH_CS_PIN, LOW);

    // Read X (command 0xD0)
    SPI.transfer(0xD0);
    delayMicroseconds(200);
    rawX = SPI.transfer(0x00);
    rawX <<= 8;
    rawX |= SPI.transfer(0x00);
    rawX >>= 3;

    // Read Y (command 0x90)
    SPI.transfer(0x90);
    delayMicroseconds(200);
    rawY = SPI.transfer(0x00);
    rawY <<= 8;
    rawY |= SPI.transfer(0x00);
    rawY >>= 3;

    digitalWrite(TOUCH_CS_PIN, HIGH);

    SPI.endTransaction();
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

    // Check if touch is valid (values in reasonable range)
    if (rawX > 100 && rawX < 4000 && rawY > 100 && rawY < 4000) {
        // Convert raw to screen coordinates using calibration
        tx = _lcd.getWidth() - TOUCH_X_FAC * rawY - TOUCH_X_OFF;
        ty = _lcd.getHeight() - TOUCH_Y_FAC * rawX - TOUCH_Y_OFF;
        return true;
    }
    return false;
}

bool WaveShare::isTouched() const {
    return _touch.isTouching();
}
