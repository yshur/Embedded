/**
 * Calculator Application - Main Entry Point
 *
 * A touch-screen calculator for the ESP32 with Waveshare 3.5" TFT LCD.
 *
 * This file contains only setup() and loop().
 * All logic is implemented in the lib/ folder classes:
 *   - WaveShare: LCD wrapper
 *   - Button: Individual button representation
 *   - Keyboard: Button grid management
 *   - CalculatorLogic: Pure math/state machine
 *   - CalculatorApp: Application controller
 *
 * Note: The original spiral demo code has been moved out of this project.
 *       This is now a clean calculator implementation.
 */

#include <Arduino.h>
#include "CalculatorApp.h"

// Global application instance
CalculatorApp app;

void setup() {
    Serial.begin(115200);
    Serial.println("Calculator App Starting...");

    // Initialize the calculator application
    app.begin();

    Serial.println("Calculator Ready!");
}

void loop() {
    // Run one iteration of the application loop
    app.loopStep();
}
