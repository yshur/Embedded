#include "CalculatorLogic.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

CalculatorLogic::CalculatorLogic()
    : _showingResult(false)
    , _isError(false)
{
    clearAll();
}

void CalculatorLogic::clearAll() {
    _expression[0] = '0';
    _expression[1] = '\0';
    _result[0] = '\0';
    _showingResult = false;
    _isError = false;
}

bool CalculatorLogic::lastCharIsOperator() const {
    size_t len = strlen(_expression);
    if (len == 0) return false;
    char last = _expression[len - 1];
    return (last == '+' || last == '-' || last == '*' || last == '/');
}

bool CalculatorLogic::lastCharIsDot() const {
    size_t len = strlen(_expression);
    if (len == 0) return false;
    return _expression[len - 1] == '.';
}

void CalculatorLogic::inputDigit(int digit) {
    if (digit < 0 || digit > 9) return;

    // If showing result, start fresh
    if (_showingResult || _isError) {
        clearAll();
    }

    size_t len = strlen(_expression);

    // Replace initial "0" with the digit (unless it's "0.")
    if (len == 1 && _expression[0] == '0') {
        _expression[0] = '0' + digit;
        return;
    }

    // Don't exceed max length
    if (len >= MAX_EXPR_LENGTH) return;

    _expression[len] = '0' + digit;
    _expression[len + 1] = '\0';
}

void CalculatorLogic::inputDot() {
    if (_showingResult || _isError) {
        clearAll();
    }

    size_t len = strlen(_expression);
    if (len >= MAX_EXPR_LENGTH) return;

    // Don't allow dot after operator - add "0." instead
    if (lastCharIsOperator() || len == 0) {
        if (len + 2 > MAX_EXPR_LENGTH) return;
        _expression[len] = '0';
        _expression[len + 1] = '.';
        _expression[len + 2] = '\0';
        return;
    }

    // Check if current number already has a dot
    // Scan backwards to find start of current number
    bool hasDot = false;
    for (int i = len - 1; i >= 0; i--) {
        char c = _expression[i];
        if (c == '+' || c == '-' || c == '*' || c == '/') break;
        if (c == '.') {
            hasDot = true;
            break;
        }
    }

    if (!hasDot) {
        _expression[len] = '.';
        _expression[len + 1] = '\0';
    }
}

void CalculatorLogic::inputOperator(char op) {
    if (op != '+' && op != '-' && op != '*' && op != '/') return;

    // If showing result, continue with that result
    if (_showingResult) {
        // Copy result to expression to continue calculating
        strncpy(_expression, _result, MAX_EXPR_LENGTH);
        _expression[MAX_EXPR_LENGTH] = '\0';
        _showingResult = false;
    }

    if (_isError) {
        clearAll();
        return;
    }

    size_t len = strlen(_expression);
    if (len >= MAX_EXPR_LENGTH) return;

    // If last char is an operator, replace it
    if (lastCharIsOperator()) {
        _expression[len - 1] = op;
        return;
    }

    // If last char is a dot, remove it first
    if (lastCharIsDot()) {
        _expression[len - 1] = op;
        return;
    }

    // Don't allow operator at start (except for initial "0")
    if (len == 0) return;

    _expression[len] = op;
    _expression[len + 1] = '\0';
}

void CalculatorLogic::backspace() {
    if (_showingResult || _isError) {
        clearAll();
        return;
    }

    size_t len = strlen(_expression);
    if (len <= 1) {
        _expression[0] = '0';
        _expression[1] = '\0';
        return;
    }

    _expression[len - 1] = '\0';
}

void CalculatorLogic::factorial() {
    // First evaluate the current expression if not already showing result
    if (!_showingResult && !_isError) {
        equals();
    }

    if (_isError) return;

    // Get the current result value
    double value = atof(_result);

    // Factorial only works for non-negative integers
    if (value < 0 || value != floor(value)) {
        strcpy(_result, "Error");
        _isError = true;
        return;
    }

    // Limit to prevent overflow (20! is near double max precision)
    if (value > 170) {
        strcpy(_result, "Error");
        _isError = true;
        return;
    }

    // Calculate factorial
    int n = (int)value;
    double fact = 1.0;
    for (int i = 2; i <= n; i++) {
        fact *= i;
    }

    formatResult(fact);
    _showingResult = true;
}

void CalculatorLogic::log2() {
    // First evaluate the current expression if not already showing result
    if (!_showingResult && !_isError) {
        equals();
    }

    if (_isError) return;

    // Get the current result value
    double value = atof(_result);

    // log2 only works for positive numbers
    if (value <= 0) {
        strcpy(_result, "Error");
        _isError = true;
        return;
    }

    // Calculate log base 2: log2(x) = ln(x) / ln(2)
    double result = log(value) / log(2.0);

    formatResult(result);
    _showingResult = true;
}

void CalculatorLogic::square() {
    // First evaluate the current expression if not already showing result
    if (!_showingResult && !_isError) {
        equals();
    }

    if (_isError) return;

    double value = atof(_result);
    double result = value * value;

    formatResult(result);
    _showingResult = true;
}

void CalculatorLogic::sine() {
    // First evaluate the current expression if not already showing result
    if (!_showingResult && !_isError) {
        equals();
    }

    if (_isError) return;

    double value = atof(_result);
    // Convert degrees to radians and calculate sin
    double result = sin(value * DEG_TO_RAD);

    formatResult(result);
    _showingResult = true;
}

void CalculatorLogic::cosine() {
    // First evaluate the current expression if not already showing result
    if (!_showingResult && !_isError) {
        equals();
    }

    if (_isError) return;

    double value = atof(_result);
    // Convert degrees to radians and calculate cos
    double result = cos(value * DEG_TO_RAD);

    formatResult(result);
    _showingResult = true;
}

void CalculatorLogic::tangent() {
    // First evaluate the current expression if not already showing result
    if (!_showingResult && !_isError) {
        equals();
    }

    if (_isError) return;

    double value = atof(_result);

    // Check for undefined tangent (90, 270, etc.)
    double normalized = fmod(value, 180.0);
    if (fabs(normalized - 90.0) < 0.0001 || fabs(normalized + 90.0) < 0.0001) {
        strcpy(_result, "Error");
        _isError = true;
        return;
    }

    // Convert degrees to radians and calculate tan
    double result = tan(value * DEG_TO_RAD);

    formatResult(result);
    _showingResult = true;
}

void CalculatorLogic::negate() {
    // For negate, we work directly on the expression or result
    if (_isError) {
        clearAll();
        return;
    }

    if (_showingResult) {
        // Negate the result
        double value = atof(_result);
        value = -value;
        formatResult(value);
    } else {
        // Negate the expression or current number
        size_t len = strlen(_expression);

        if (len == 0) return;

        // If expression starts with minus, remove it
        if (_expression[0] == '-') {
            memmove(_expression, _expression + 1, len);
        }
        // If expression is just "0", do nothing
        else if (len == 1 && _expression[0] == '0') {
            return;
        }
        // Otherwise, add minus at the beginning
        else {
            if (len < MAX_EXPR_LENGTH) {
                memmove(_expression + 1, _expression, len + 1);
                _expression[0] = '-';
            }
        }
    }
}

void CalculatorLogic::equals() {
    if (_showingResult) return;  // Already showing result
    if (_isError) {
        clearAll();
        return;
    }

    // Remove trailing operator if any
    size_t len = strlen(_expression);
    while (len > 0 && lastCharIsOperator()) {
        _expression[len - 1] = '\0';
        len--;
    }

    if (len == 0) {
        clearAll();
        return;
    }

    bool success = true;
    double result = evaluate(_expression, success);

    if (success) {
        formatResult(result);
        _showingResult = true;
    } else {
        strcpy(_result, "Error");
        _isError = true;
        _showingResult = true;
    }
}

const char* CalculatorLogic::getDisplayValue() const {
    if (_showingResult || _isError) {
        return _result;
    }
    return _expression;
}

void CalculatorLogic::formatResult(double value) {
    // Handle special values
    if (isnan(value) || isinf(value)) {
        strcpy(_result, "Error");
        _isError = true;
        return;
    }

    // Format the number
    if (value == floor(value) && fabs(value) < 1e10) {
        snprintf(_result, MAX_EXPR_LENGTH, "%.0f", value);
    } else if (fabs(value) >= 1e10 || (fabs(value) < 1e-6 && value != 0)) {
        snprintf(_result, MAX_EXPR_LENGTH, "%.4e", value);
    } else {
        snprintf(_result, MAX_EXPR_LENGTH, "%.8g", value);
    }
}

// ============ Expression Parser with Operator Precedence ============
// Uses recursive descent parsing:
// Expression = Term (('+' | '-') Term)*
// Term = Number (('*' | '/') Number)*
// Number = [0-9]+ ('.' [0-9]+)?

double CalculatorLogic::evaluate(const char* expr, bool& success) {
    const char* p = expr;
    success = true;
    double result = parseExpression(p, success);

    // Check if we consumed the entire expression
    if (success && *p != '\0') {
        success = false;
    }

    return result;
}

// Skip whitespace (shouldn't be any, but just in case)
static void skipSpaces(const char*& p) {
    while (*p == ' ') p++;
}

double CalculatorLogic::parseNumber(const char*& p, bool& success) {
    skipSpaces(p);

    if (!success) return 0;

    // Handle negative numbers at start or after operator
    bool negative = false;
    if (*p == '-') {
        negative = true;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (!isdigit(*p) && *p != '.') {
        success = false;
        return 0;
    }

    double value = 0;

    // Integer part
    while (isdigit(*p)) {
        value = value * 10 + (*p - '0');
        p++;
    }

    // Decimal part
    if (*p == '.') {
        p++;
        double decimal = 0.1;
        while (isdigit(*p)) {
            value += (*p - '0') * decimal;
            decimal *= 0.1;
            p++;
        }
    }

    return negative ? -value : value;
}

double CalculatorLogic::parseTerm(const char*& p, bool& success) {
    double left = parseNumber(p, success);
    if (!success) return 0;

    skipSpaces(p);

    while (*p == '*' || *p == '/') {
        char op = *p;
        p++;
        skipSpaces(p);

        double right = parseNumber(p, success);
        if (!success) return 0;

        if (op == '*') {
            left *= right;
        } else {
            if (right == 0) {
                success = false;  // Division by zero
                return 0;
            }
            left /= right;
        }

        skipSpaces(p);
    }

    return left;
}

double CalculatorLogic::parseExpression(const char*& p, bool& success) {
    skipSpaces(p);

    double left = parseTerm(p, success);
    if (!success) return 0;

    skipSpaces(p);

    while (*p == '+' || *p == '-') {
        char op = *p;
        p++;
        skipSpaces(p);

        double right = parseTerm(p, success);
        if (!success) return 0;

        if (op == '+') {
            left += right;
        } else {
            left -= right;
        }

        skipSpaces(p);
    }

    return left;
}
