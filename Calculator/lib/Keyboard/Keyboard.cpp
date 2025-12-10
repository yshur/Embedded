#include "Keyboard.h"
#include "WaveShare.h"

// Color definitions for buttons
namespace ButtonColors {
    // Digit buttons - light gray
    constexpr uint16_t DIGIT_BG = 0xDEFB;      // Light gray
    constexpr uint16_t DIGIT_FG = Colors::BLACK;
    constexpr uint16_t DIGIT_PRESSED = 0xBDF7; // Darker gray

    // Operator buttons - orange
    constexpr uint16_t OP_BG = 0xFD20;         // Orange
    constexpr uint16_t OP_FG = Colors::WHITE;
    constexpr uint16_t OP_PRESSED = 0xFB00;    // Darker orange

    // Function buttons - dark gray
    constexpr uint16_t FUNC_BG = 0xA514;       // Dark gray
    constexpr uint16_t FUNC_FG = Colors::WHITE;
    constexpr uint16_t FUNC_PRESSED = 0x7BEF;  // Even darker

    // Clear button - red
    constexpr uint16_t CLEAR_BG = 0xF800;      // Red
    constexpr uint16_t CLEAR_FG = Colors::WHITE;
    constexpr uint16_t CLEAR_PRESSED = 0xB000; // Dark red

    // Equals button - green
    constexpr uint16_t EQUALS_BG = 0x07E0;     // Green
    constexpr uint16_t EQUALS_FG = Colors::WHITE;
    constexpr uint16_t EQUALS_PRESSED = 0x05C0;// Dark green
}

Keyboard::Keyboard()
    : _buttonWidth(0)
    , _buttonHeight(0)
    , _topMargin(0)
    , _padding(2)
{
}

void Keyboard::initLayout(int16_t screenWidth, int16_t screenHeight, int16_t topMargin) {
    _topMargin = topMargin;

    // Calculate button size to fill the remaining screen
    int16_t availableHeight = screenHeight - topMargin;
    _buttonWidth = (screenWidth - _padding * (COLS + 1)) / COLS;
    _buttonHeight = (availableHeight - _padding * (ROWS + 1)) / ROWS;

    using namespace ButtonColors;

    // Row 0: C, √, x², /
    createButton(0, 0, "C",  KeyType::CLEAR,    CLEAR_BG, CLEAR_FG, CLEAR_PRESSED);
    createButton(0, 1, "^",  KeyType::UNARY,    FUNC_BG,  FUNC_FG,  FUNC_PRESSED);  // sqrt symbol
    createButton(0, 2, "x2", KeyType::UNARY,    FUNC_BG,  FUNC_FG,  FUNC_PRESSED);  // x squared
    createButton(0, 3, "/",  KeyType::OPERATOR, OP_BG,    OP_FG,    OP_PRESSED);

    // Row 1: 7, 8, 9, *
    createButton(1, 0, "7",  KeyType::DIGIT,    DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(1, 1, "8",  KeyType::DIGIT,    DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(1, 2, "9",  KeyType::DIGIT,    DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(1, 3, "*",  KeyType::OPERATOR, OP_BG,    OP_FG,    OP_PRESSED);

    // Row 2: 4, 5, 6, -
    createButton(2, 0, "4",  KeyType::DIGIT,    DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(2, 1, "5",  KeyType::DIGIT,    DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(2, 2, "6",  KeyType::DIGIT,    DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(2, 3, "-",  KeyType::OPERATOR, OP_BG,    OP_FG,    OP_PRESSED);

    // Row 3: 1, 2, 3, +
    createButton(3, 0, "1",  KeyType::DIGIT,    DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(3, 1, "2",  KeyType::DIGIT,    DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(3, 2, "3",  KeyType::DIGIT,    DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(3, 3, "+",  KeyType::OPERATOR, OP_BG,    OP_FG,    OP_PRESSED);

    // Row 4: sin, 0, ., =
    createButton(4, 0, "sin", KeyType::UNARY,   FUNC_BG,  FUNC_FG,  FUNC_PRESSED);
    createButton(4, 1, "0",   KeyType::DIGIT,   DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(4, 2, ".",   KeyType::DOT,     DIGIT_BG, DIGIT_FG, DIGIT_PRESSED);
    createButton(4, 3, "=",   KeyType::EQUALS,  EQUALS_BG, EQUALS_FG, EQUALS_PRESSED);

    // Row 5: cos, tan, +/-, %
    createButton(5, 0, "cos", KeyType::UNARY,    FUNC_BG,  FUNC_FG,  FUNC_PRESSED);
    createButton(5, 1, "tan", KeyType::UNARY,    FUNC_BG,  FUNC_FG,  FUNC_PRESSED);
    createButton(5, 2, "+/-", KeyType::UNARY,    FUNC_BG,  FUNC_FG,  FUNC_PRESSED);
    createButton(5, 3, "%",   KeyType::OPERATOR, OP_BG,    OP_FG,    OP_PRESSED);
}

void Keyboard::createButton(int row, int col, const char* label, KeyType type,
                             uint16_t bgColor, uint16_t fgColor, uint16_t pressedColor) {
    int index = row * COLS + col;
    if (index >= NUM_BUTTONS) return;

    int16_t x = _padding + col * (_buttonWidth + _padding);
    int16_t y = _topMargin + _padding + row * (_buttonHeight + _padding);

    _buttons[index].init(label, type, x, y, _buttonWidth, _buttonHeight,
                         bgColor, fgColor, pressedColor);
}

void Keyboard::drawAll(WaveShare& lcd) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (_buttons[i].isValid()) {
            _buttons[i].draw(lcd, false);
        }
    }
}

void Keyboard::drawButton(WaveShare& lcd, int index, bool pressed) {
    if (index >= 0 && index < NUM_BUTTONS && _buttons[index].isValid()) {
        _buttons[index].draw(lcd, pressed);
    }
}

int Keyboard::hitTest(int16_t tx, int16_t ty) const {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (_buttons[i].hitTest(tx, ty)) {
            return i;
        }
    }
    return -1;
}

const Button& Keyboard::getButton(int index) const {
    static Button emptyButton;
    if (index >= 0 && index < NUM_BUTTONS) {
        return _buttons[index];
    }
    return emptyButton;
}

Button& Keyboard::getButton(int index) {
    static Button emptyButton;
    if (index >= 0 && index < NUM_BUTTONS) {
        return _buttons[index];
    }
    return emptyButton;
}
