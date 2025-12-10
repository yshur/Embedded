#include "Button.h"
#include "WaveShare.h"
#include "fonts/fonts.h"

Button::Button()
    : _type(KeyType::NONE)
    , _x(0), _y(0), _w(0), _h(0)
    , _bgColor(0), _fgColor(0), _pressedColor(0)
{
    _label[0] = '\0';
}

Button::Button(const char* label, KeyType type,
               int16_t x, int16_t y, int16_t w, int16_t h,
               uint16_t bgColor, uint16_t fgColor, uint16_t pressedColor)
{
    init(label, type, x, y, w, h, bgColor, fgColor, pressedColor);
}

void Button::init(const char* label, KeyType type,
                  int16_t x, int16_t y, int16_t w, int16_t h,
                  uint16_t bgColor, uint16_t fgColor, uint16_t pressedColor) {
    // Copy label (truncate if too long)
    strncpy(_label, label, sizeof(_label) - 1);
    _label[sizeof(_label) - 1] = '\0';

    _type = type;
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _bgColor = bgColor;
    _fgColor = fgColor;
    _pressedColor = pressedColor;
}

void Button::draw(WaveShare& lcd, bool pressed) const {
    if (_type == KeyType::NONE) return;

    // Choose background color based on pressed state
    uint16_t bg = pressed ? _pressedColor : _bgColor;

    // Draw button background
    lcd.fillRect(_x, _y, _w, _h, bg);

    // Draw button border
    lcd.drawRect(_x, _y, _w, _h, Colors::BLACK);

    // Draw label centered in button
    lcd.drawTextCentered(_x, _y, _w, _h, _label, &Font20, bg, _fgColor);
}

bool Button::hitTest(int16_t tx, int16_t ty) const {
    if (_type == KeyType::NONE) return false;

    return (tx >= _x && tx < _x + _w &&
            ty >= _y && ty < _y + _h);
}

int8_t Button::getDigitValue() const {
    if (_type != KeyType::DIGIT) return -1;

    // Label should be a single digit character
    if (_label[0] >= '0' && _label[0] <= '9') {
        return _label[0] - '0';
    }
    return -1;
}

char Button::getOperator() const {
    if (_type != KeyType::OPERATOR) return '\0';

    // Return the first character of the label
    return _label[0];
}
