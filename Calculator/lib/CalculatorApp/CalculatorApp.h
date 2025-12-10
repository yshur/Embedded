#ifndef CALCULATOR_APP_H
#define CALCULATOR_APP_H

#include <Arduino.h>
#include "WaveShare.h"
#include "Keyboard.h"
#include "CalculatorLogic.h"

/**
 * CalculatorApp - Main application controller
 *
 * Ties together the LCD, keyboard, and calculator logic.
 * Handles UI rendering and touch interaction.
 */
class CalculatorApp {
public:
    // Display area dimensions
    static constexpr int16_t DISPLAY_HEIGHT = 60;
    static constexpr int16_t DISPLAY_MARGIN = 5;

    CalculatorApp();
    ~CalculatorApp() = default;

    // Initialization - must be called in setup()
    void begin();

    // Main loop step - call this in loop()
    void loopStep();

    // Get references to components (for advanced use)
    WaveShare& getLCD() { return _lcd; }
    Keyboard& getKeyboard() { return _keyboard; }
    CalculatorLogic& getLogic() { return _logic; }

private:
    WaveShare _lcd;
    Keyboard _keyboard;
    CalculatorLogic _logic;

    // Touch state
    int _lastPressedButton;
    bool _waitingForRelease;
    unsigned long _lastTouchTime;

    // Display colors
    static constexpr uint16_t DISPLAY_BG = Colors::BLACK;
    static constexpr uint16_t DISPLAY_FG = Colors::WHITE;
    static constexpr uint16_t SCREEN_BG = 0x2104;  // Dark gray

    // Debounce time in milliseconds
    static constexpr unsigned long TOUCH_DEBOUNCE_MS = 50;

    // UI drawing
    void drawUI();
    void drawDisplay();
    void drawDisplayArea();

    // Touch handling
    void handleTouch(int16_t tx, int16_t ty);
    void processButtonPress(int buttonIndex);

    // Button press animation
    void animateButtonPress(int buttonIndex);
};

#endif // CALCULATOR_APP_H
