#ifndef CALCULATOR_LOGIC_H
#define CALCULATOR_LOGIC_H

#include <Arduino.h>

/**
 * CalculatorState - Internal state machine states
 */
enum class CalculatorState {
    FIRST_OPERAND,    // Entering first number
    OPERATOR_ENTERED, // Operator selected, waiting for second operand
    SECOND_OPERAND,   // Entering second number
    RESULT_SHOWN,     // Result displayed, next input starts fresh
    ERROR             // Error state (e.g., division by zero)
};

/**
 * BinaryOp - Supported binary operations
 */
enum class BinaryOp {
    NONE,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULO
};

/**
 * CalculatorLogic - Pure calculator state machine
 *
 * This class handles all calculator logic without any LCD dependencies.
 * It manages operands, operations, and the state machine.
 */
class CalculatorLogic {
public:
    // Maximum digits for input display
    static constexpr int MAX_DISPLAY_LENGTH = 16;

    CalculatorLogic();
    ~CalculatorLogic() = default;

    // Input operations
    void inputDigit(int digit);          // 0-9
    void inputDot();                      // Decimal point

    // Binary operations
    void applyBinaryOp(BinaryOp op);     // Set pending operation
    void applyBinaryOp(char opChar);     // Convenience: '+', '-', '*', '/', '%'

    // Equals - execute pending operation
    void equals();

    // Clear
    void clearAll();                      // Full reset
    void clearEntry();                    // Clear current entry only

    // Unary operations
    void square();                        // x²
    void squareRoot();                    // √x
    void sine();                          // sin(x) in degrees
    void cosine();                        // cos(x) in degrees
    void tangent();                       // tan(x) in degrees
    void negate();                        // +/-
    void percent();                       // %

    // Display
    const char* getDisplayValue() const;
    bool isError() const { return _state == CalculatorState::ERROR; }

    // State inspection (for debugging/testing)
    CalculatorState getState() const { return _state; }
    double getOperand1() const { return _operand1; }
    double getOperand2() const { return _operand2; }
    BinaryOp getPendingOp() const { return _pendingOp; }

private:
    double _operand1;                     // First operand
    double _operand2;                     // Second operand
    BinaryOp _pendingOp;                  // Pending binary operation
    char _currentInput[MAX_DISPLAY_LENGTH + 1]; // Current input string
    mutable char _displayBuffer[MAX_DISPLAY_LENGTH + 1]; // For getDisplayValue()
    CalculatorState _state;               // Current state
    bool _hasDecimal;                     // Current input has decimal point

    // Helper methods
    void resetInput();
    double getCurrentInputValue() const;
    void setDisplayFromValue(double value);
    double executeOperation(double a, double b, BinaryOp op);
    void setError();
    void appendToInput(char c);
};

#endif // CALCULATOR_LOGIC_H
