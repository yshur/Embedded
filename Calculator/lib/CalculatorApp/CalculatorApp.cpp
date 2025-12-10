#include "CalculatorApp.h"
#include "Button.h"
#include "fonts/fonts.h"

CalculatorApp::CalculatorApp()
    : _lastPressedButton(-1)
    , _waitingForRelease(false)
    , _lastTouchTime(0)
{
}

void CalculatorApp::begin() {
    // Initialize LCD and touch
    _lcd.begin();

    // Clear screen with background color
    _lcd.fillScreen(SCREEN_BG);

    // Initialize keyboard layout
    // Top margin = display height + margins
    int16_t topMargin = DISPLAY_HEIGHT + DISPLAY_MARGIN * 2;
    _keyboard.initLayout(_lcd.getWidth(), _lcd.getHeight(), topMargin);

    // Draw the full UI
    drawUI();

    // Reset calculator state
    _logic.clearAll();

    // Initial display update
    drawDisplay();
}

void CalculatorApp::drawUI() {
    // Draw display area background
    drawDisplayArea();

    // Draw all keyboard buttons
    _keyboard.drawAll(_lcd);
}

void CalculatorApp::drawDisplayArea() {
    // Draw display background
    _lcd.fillRect(DISPLAY_MARGIN, DISPLAY_MARGIN,
                  _lcd.getWidth() - DISPLAY_MARGIN * 2,
                  DISPLAY_HEIGHT,
                  DISPLAY_BG);

    // Draw display border
    _lcd.drawRect(DISPLAY_MARGIN, DISPLAY_MARGIN,
                  _lcd.getWidth() - DISPLAY_MARGIN * 2,
                  DISPLAY_HEIGHT,
                  Colors::WHITE);
}

void CalculatorApp::drawDisplay() {
    // Get current display value
    const char* value = _logic.getDisplayValue();

    // Calculate display area
    int16_t displayX = DISPLAY_MARGIN + 5;
    int16_t displayY = DISPLAY_MARGIN + 5;
    int16_t displayW = _lcd.getWidth() - DISPLAY_MARGIN * 2 - 10;
    int16_t displayH = DISPLAY_HEIGHT - 10;

    // Clear the display area (keep border intact)
    _lcd.fillRect(displayX, displayY, displayW, displayH, DISPLAY_BG);

    // Choose font based on text length
    sFONT* font = &Font24;
    size_t len = strlen(value);
    if (len > 16) {
        font = &Font16;
    } else if (len > 12) {
        font = &Font20;
    }

    // Draw text right-aligned with some padding from the right edge
    int16_t textY = DISPLAY_MARGIN + (DISPLAY_HEIGHT - font->Height) / 2;
    _lcd.drawTextRightAligned(displayX, textY, displayW - 5, value, font,
                               DISPLAY_BG, DISPLAY_FG);
}

void CalculatorApp::loopStep() {
    int16_t tx, ty;

    // Check for touch
    if (_lcd.readTouch(tx, ty)) {
        // Debounce
        unsigned long now = millis();
        if (now - _lastTouchTime < TOUCH_DEBOUNCE_MS) {
            return;
        }
        _lastTouchTime = now;

        if (!_waitingForRelease) {
            handleTouch(tx, ty);
        }
    } else {
        // Touch released
        if (_waitingForRelease && _lastPressedButton >= 0) {
            // Redraw button in normal state
            _keyboard.drawButton(_lcd, _lastPressedButton, false);
            _lastPressedButton = -1;
        }
        _waitingForRelease = false;
    }
}

void CalculatorApp::handleTouch(int16_t tx, int16_t ty) {
    // Hit test against keyboard
    int buttonIndex = _keyboard.hitTest(tx, ty);

    if (buttonIndex >= 0) {
        // Animate button press
        animateButtonPress(buttonIndex);

        // Process the button
        processButtonPress(buttonIndex);

        // Update display
        drawDisplay();

        // Mark that we're waiting for release
        _waitingForRelease = true;
        _lastPressedButton = buttonIndex;
    }
}

void CalculatorApp::animateButtonPress(int buttonIndex) {
    // Draw button in pressed state
    _keyboard.drawButton(_lcd, buttonIndex, true);
}

void CalculatorApp::processButtonPress(int buttonIndex) {
    const Button& btn = _keyboard.getButton(buttonIndex);
    if (!btn.isValid()) return;

    KeyType type = btn.getType();
    const char* label = btn.getLabel();

    switch (type) {
        case KeyType::DIGIT: {
            int8_t digit = btn.getDigitValue();
            if (digit >= 0) {
                _logic.inputDigit(digit);
            }
            break;
        }

        case KeyType::DOT:
            _logic.inputDot();
            break;

        case KeyType::OPERATOR: {
            char op = btn.getOperator();
            if (op != '\0') {
                _logic.applyBinaryOp(op);
            }
            break;
        }

        case KeyType::EQUALS:
            _logic.equals();
            break;

        case KeyType::CLEAR:
            _logic.clearAll();
            break;

        case KeyType::UNARY:
            // Handle unary functions based on label
            if (strcmp(label, "x2") == 0) {
                _logic.square();
            } else if (strcmp(label, "^") == 0) {  // sqrt symbol
                _logic.squareRoot();
            } else if (strcmp(label, "sin") == 0) {
                _logic.sine();
            } else if (strcmp(label, "cos") == 0) {
                _logic.cosine();
            } else if (strcmp(label, "tan") == 0) {
                _logic.tangent();
            } else if (strcmp(label, "+/-") == 0) {
                _logic.negate();
            }
            break;

        case KeyType::NONE:
        default:
            break;
    }
}
