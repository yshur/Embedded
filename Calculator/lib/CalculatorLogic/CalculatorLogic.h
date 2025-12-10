#ifndef CALCULATOR_LOGIC_H
#define CALCULATOR_LOGIC_H

#include <Arduino.h>

/**
 * CalculatorLogic - Expression-based calculator
 *
 * Builds an expression string (e.g., "9*8/9") and evaluates it
 * with proper operator precedence when equals is pressed.
 */
class CalculatorLogic {
public:
    // Maximum expression length
    static constexpr int MAX_EXPR_LENGTH = 32;

    CalculatorLogic();
    ~CalculatorLogic() = default;

    // Input operations
    void inputDigit(int digit);          // 0-9
    void inputDot();                      // Decimal point
    void inputOperator(char op);          // +, -, *, /

    // Equals - evaluate the expression
    void equals();

    // Clear
    void clearAll();                      // Full reset
    void backspace();                     // Delete last character

    // Unary operations (apply to current result/value)
    void factorial();                     // n!
    void log2();                          // log base 2
    void square();                        // x²
    void sine();                          // sin(x) in degrees
    void cosine();                        // cos(x) in degrees
    void tangent();                       // tan(x) in degrees
    void negate();                        // +/- toggle sign

    // Display - returns the current expression or result
    const char* getDisplayValue() const;

    // Check if showing result (after equals)
    bool isShowingResult() const { return _showingResult; }
    bool isError() const { return _isError; }

private:
    char _expression[MAX_EXPR_LENGTH + 1];  // The expression string
    char _result[MAX_EXPR_LENGTH + 1];      // Result after evaluation
    bool _showingResult;                     // True after equals pressed
    bool _isError;                           // True if evaluation failed

    // Expression evaluation with operator precedence
    double evaluate(const char* expr, bool& success);
    double parseNumber(const char*& p, bool& success);
    double parseTerm(const char*& p, bool& success);      // Handles * and /
    double parseExpression(const char*& p, bool& success); // Handles + and -

    // Helper to format result
    void formatResult(double value);

    // Check if last char is an operator
    bool lastCharIsOperator() const;
    bool lastCharIsDot() const;
};

#endif // CALCULATOR_LOGIC_H
