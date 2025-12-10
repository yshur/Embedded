#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <Arduino.h>
#include "Button.h"

// Forward declaration
class WaveShare;

/**
 * Keyboard - Manages all calculator buttons
 *
 * Creates a 6x4 grid of buttons including:
 * - Numeric keys (0-9)
 * - Operators (+, -, *, /)
 * - Equals (=)
 * - Clear (C)
 * - Decimal point (.)
 * - Unary functions (x², √)
 */
class Keyboard {
public:
    // Grid dimensions
    static constexpr int ROWS = 6;
    static constexpr int COLS = 4;
    static constexpr int NUM_BUTTONS = ROWS * COLS;

    Keyboard();
    ~Keyboard() = default;

    // Initialize button layout based on screen dimensions
    void initLayout(int16_t screenWidth, int16_t screenHeight, int16_t topMargin);

    // Draw all buttons
    void drawAll(WaveShare& lcd);

    // Draw a specific button (by index)
    void drawButton(WaveShare& lcd, int index, bool pressed = false);

    // Hit test - returns button index or -1 if no button hit
    int hitTest(int16_t tx, int16_t ty) const;

    // Get button by index
    const Button& getButton(int index) const;
    Button& getButton(int index);

    // Get button dimensions (for external calculations)
    int16_t getButtonWidth() const { return _buttonWidth; }
    int16_t getButtonHeight() const { return _buttonHeight; }
    int16_t getTopMargin() const { return _topMargin; }

private:
    Button _buttons[NUM_BUTTONS];
    int16_t _buttonWidth;
    int16_t _buttonHeight;
    int16_t _topMargin;
    int16_t _padding;

    // Helper to create a button at grid position
    void createButton(int row, int col, const char* label, KeyType type,
                      uint16_t bgColor, uint16_t fgColor, uint16_t pressedColor);
};

#endif // KEYBOARD_H
