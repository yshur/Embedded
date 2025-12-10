#include "WaveShare.h"

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
    if (_touch.scan()) {
        if (_touch.isPressed()) {
            tx = _touch.getX();
            ty = _touch.getY();
            return true;
        }
    }
    return false;
}

bool WaveShare::isTouched() const {
    return _touch.isTouching();
}
