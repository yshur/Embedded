#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

// Forward declaration
class WaveShare;

/**
 * KeyType - Categorizes button types for the calculator
 */
enum class KeyType {
    DIGIT,      // 0-9
    DOT,        // Decimal point
    OPERATOR,   // + - * /
    EQUALS,     // =
    CLEAR,      // C/AC
    UNARY,      // x², √, sin, etc.
    BACKSPACE,  // Delete last character
    NONE        // Placeholder/empty
};

/**
 * Button - Represents a single on-screen calculator button
 *
 * Handles its own drawing and hit-testing.
 */
class Button {
public:
    // Default constructor (creates an empty button)
    Button();

    // Full constructor
    Button(const char* label, KeyType type,
           int16_t x, int16_t y, int16_t w, int16_t h,
           uint16_t bgColor, uint16_t fgColor, uint16_t pressedColor);

    // Initialize/update button properties
    void init(const char* label, KeyType type,
              int16_t x, int16_t y, int16_t w, int16_t h,
              uint16_t bgColor, uint16_t fgColor, uint16_t pressedColor);

    // Drawing
    void draw(WaveShare& lcd, bool pressed = false) const;

    // Hit testing - returns true if (tx, ty) is within button bounds
    bool hitTest(int16_t tx, int16_t ty) const;

    // Getters
    const char* getLabel() const { return _label; }
    KeyType getType() const { return _type; }
    int16_t getX() const { return _x; }
    int16_t getY() const { return _y; }
    int16_t getWidth() const { return _w; }
    int16_t getHeight() const { return _h; }

    // Check if button is valid/initialized
    bool isValid() const { return _type != KeyType::NONE; }

    // Get the numeric value for digit buttons (0-9)
    // Returns -1 for non-digit buttons
    int8_t getDigitValue() const;

    // Get the operator character for operator buttons
    // Returns '\0' for non-operator buttons
    char getOperator() const;

private:
    char _label[8];          // Button label (short text)
    KeyType _type;           // Button type
    int16_t _x, _y;          // Position
    int16_t _w, _h;          // Size
    uint16_t _bgColor;       // Normal background color
    uint16_t _fgColor;       // Text color
    uint16_t _pressedColor;  // Pressed background color
};

#endif // BUTTON_H
