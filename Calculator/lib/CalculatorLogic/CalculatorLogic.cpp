#include "CalculatorLogic.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

// Note: DEG_TO_RAD is already defined in Arduino.h

CalculatorLogic::CalculatorLogic()
    : _operand1(0.0)
    , _operand2(0.0)
    , _pendingOp(BinaryOp::NONE)
    , _state(CalculatorState::FIRST_OPERAND)
    , _hasDecimal(false)
{
    resetInput();
}

void CalculatorLogic::resetInput() {
    _currentInput[0] = '0';
    _currentInput[1] = '\0';
    _hasDecimal = false;
}

double CalculatorLogic::getCurrentInputValue() const {
    return atof(_currentInput);
}

void CalculatorLogic::setDisplayFromValue(double value) {
    // Handle special values
    if (isnan(value) || isinf(value)) {
        strcpy(_currentInput, "Error");
        _state = CalculatorState::ERROR;
        return;
    }

    // Format the number
    // Try to display as integer if possible
    if (value == floor(value) && fabs(value) < 1e10) {
        snprintf(_currentInput, MAX_DISPLAY_LENGTH, "%.0f", value);
    } else {
        // Use scientific notation for very large/small numbers
        if (fabs(value) >= 1e10 || (fabs(value) < 1e-6 && value != 0)) {
            snprintf(_currentInput, MAX_DISPLAY_LENGTH, "%.4e", value);
        } else {
            snprintf(_currentInput, MAX_DISPLAY_LENGTH, "%.8g", value);
        }
    }

    _hasDecimal = (strchr(_currentInput, '.') != nullptr);
}

void CalculatorLogic::appendToInput(char c) {
    size_t len = strlen(_currentInput);

    // Don't exceed max length
    if (len >= MAX_DISPLAY_LENGTH - 1) return;

    // Handle leading zero replacement
    if (len == 1 && _currentInput[0] == '0' && c != '.') {
        _currentInput[0] = c;
        return;
    }

    _currentInput[len] = c;
    _currentInput[len + 1] = '\0';
}

void CalculatorLogic::inputDigit(int digit) {
    if (digit < 0 || digit > 9) return;

    // Clear error state on new input
    if (_state == CalculatorState::ERROR) {
        clearAll();
    }

    // After showing result, start fresh
    if (_state == CalculatorState::RESULT_SHOWN) {
        clearAll();
    }

    // After operator, start new input
    if (_state == CalculatorState::OPERATOR_ENTERED) {
        resetInput();
        _state = CalculatorState::SECOND_OPERAND;
    }

    appendToInput('0' + digit);
}

void CalculatorLogic::inputDot() {
    if (_state == CalculatorState::ERROR) {
        clearAll();
    }

    if (_state == CalculatorState::RESULT_SHOWN) {
        clearAll();
    }

    if (_state == CalculatorState::OPERATOR_ENTERED) {
        resetInput();
        _state = CalculatorState::SECOND_OPERAND;
    }

    // Only add decimal if not already present
    if (!_hasDecimal) {
        appendToInput('.');
        _hasDecimal = true;
    }
}

void CalculatorLogic::applyBinaryOp(BinaryOp op) {
    if (_state == CalculatorState::ERROR) return;

    // If we already have a pending operation and second operand, execute first
    if (_state == CalculatorState::SECOND_OPERAND && _pendingOp != BinaryOp::NONE) {
        equals();
        if (_state == CalculatorState::ERROR) return;
    }

    // Store current value as operand1
    _operand1 = getCurrentInputValue();
    _pendingOp = op;
    _state = CalculatorState::OPERATOR_ENTERED;
}

void CalculatorLogic::applyBinaryOp(char opChar) {
    BinaryOp op = BinaryOp::NONE;

    switch (opChar) {
        case '+': op = BinaryOp::ADD; break;
        case '-': op = BinaryOp::SUBTRACT; break;
        case '*': op = BinaryOp::MULTIPLY; break;
        case '/': op = BinaryOp::DIVIDE; break;
        case '%': op = BinaryOp::MODULO; break;
        default: return;
    }

    applyBinaryOp(op);
}

double CalculatorLogic::executeOperation(double a, double b, BinaryOp op) {
    switch (op) {
        case BinaryOp::ADD:      return a + b;
        case BinaryOp::SUBTRACT: return a - b;
        case BinaryOp::MULTIPLY: return a * b;
        case BinaryOp::DIVIDE:
            if (b == 0.0) {
                setError();
                return 0.0;
            }
            return a / b;
        case BinaryOp::MODULO:
            if (b == 0.0) {
                setError();
                return 0.0;
            }
            return fmod(a, b);
        default:
            return a;
    }
}

void CalculatorLogic::equals() {
    if (_state == CalculatorState::ERROR) return;

    if (_pendingOp == BinaryOp::NONE) {
        // No operation pending, just show current value
        _state = CalculatorState::RESULT_SHOWN;
        return;
    }

    // Get second operand
    _operand2 = getCurrentInputValue();

    // Execute operation
    double result = executeOperation(_operand1, _operand2, _pendingOp);

    if (_state != CalculatorState::ERROR) {
        setDisplayFromValue(result);
        _operand1 = result;  // Store for chained operations
        _pendingOp = BinaryOp::NONE;
        _state = CalculatorState::RESULT_SHOWN;
    }
}

void CalculatorLogic::clearAll() {
    _operand1 = 0.0;
    _operand2 = 0.0;
    _pendingOp = BinaryOp::NONE;
    _state = CalculatorState::FIRST_OPERAND;
    resetInput();
}

void CalculatorLogic::clearEntry() {
    resetInput();
}

void CalculatorLogic::setError() {
    strcpy(_currentInput, "Error");
    _state = CalculatorState::ERROR;
}

// Unary operations

void CalculatorLogic::square() {
    if (_state == CalculatorState::ERROR) return;

    double value = getCurrentInputValue();
    double result = value * value;
    setDisplayFromValue(result);

    if (_state == CalculatorState::FIRST_OPERAND ||
        _state == CalculatorState::RESULT_SHOWN) {
        _operand1 = result;
    }
    _state = CalculatorState::RESULT_SHOWN;
}

void CalculatorLogic::squareRoot() {
    if (_state == CalculatorState::ERROR) return;

    double value = getCurrentInputValue();
    if (value < 0) {
        setError();
        return;
    }

    double result = sqrt(value);
    setDisplayFromValue(result);

    if (_state == CalculatorState::FIRST_OPERAND ||
        _state == CalculatorState::RESULT_SHOWN) {
        _operand1 = result;
    }
    _state = CalculatorState::RESULT_SHOWN;
}

void CalculatorLogic::sine() {
    if (_state == CalculatorState::ERROR) return;

    double value = getCurrentInputValue();
    double result = sin(value * DEG_TO_RAD);
    setDisplayFromValue(result);

    if (_state == CalculatorState::FIRST_OPERAND ||
        _state == CalculatorState::RESULT_SHOWN) {
        _operand1 = result;
    }
    _state = CalculatorState::RESULT_SHOWN;
}

void CalculatorLogic::cosine() {
    if (_state == CalculatorState::ERROR) return;

    double value = getCurrentInputValue();
    double result = cos(value * DEG_TO_RAD);
    setDisplayFromValue(result);

    if (_state == CalculatorState::FIRST_OPERAND ||
        _state == CalculatorState::RESULT_SHOWN) {
        _operand1 = result;
    }
    _state = CalculatorState::RESULT_SHOWN;
}

void CalculatorLogic::tangent() {
    if (_state == CalculatorState::ERROR) return;

    double value = getCurrentInputValue();
    // Check for undefined tangent (90, 270, etc.)
    double normalized = fmod(value, 180.0);
    if (fabs(normalized - 90.0) < 0.0001 || fabs(normalized + 90.0) < 0.0001) {
        setError();
        return;
    }

    double result = tan(value * DEG_TO_RAD);
    setDisplayFromValue(result);

    if (_state == CalculatorState::FIRST_OPERAND ||
        _state == CalculatorState::RESULT_SHOWN) {
        _operand1 = result;
    }
    _state = CalculatorState::RESULT_SHOWN;
}

void CalculatorLogic::negate() {
    if (_state == CalculatorState::ERROR) return;

    // Handle the special case of "0"
    if (strcmp(_currentInput, "0") == 0) return;

    // Toggle sign
    if (_currentInput[0] == '-') {
        // Remove minus sign
        memmove(_currentInput, _currentInput + 1, strlen(_currentInput));
    } else {
        // Add minus sign
        size_t len = strlen(_currentInput);
        if (len < MAX_DISPLAY_LENGTH - 1) {
            memmove(_currentInput + 1, _currentInput, len + 1);
            _currentInput[0] = '-';
        }
    }
}

void CalculatorLogic::percent() {
    if (_state == CalculatorState::ERROR) return;

    double value = getCurrentInputValue();
    double result = value / 100.0;
    setDisplayFromValue(result);

    if (_state == CalculatorState::FIRST_OPERAND ||
        _state == CalculatorState::RESULT_SHOWN) {
        _operand1 = result;
    }
}

const char* CalculatorLogic::getDisplayValue() const {
    // Return current input buffer
    return _currentInput;
}
