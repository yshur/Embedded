#include <Arduino.h>

// PWM LED on GPIO4
#define LED_PIN 4

void setup() {
  // Configure LED pin as OUTPUT for PWM
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Loop from 0% to 100% duty cycle in 10% steps
  // analogWrite uses 0-255 range, so 10% = 25.5 ≈ 26
  for (int percent = 0; percent <= 100; percent += 10) {
    // Convert percentage to 0-255 range
    int pwmValue = (percent * 255) / 100;

    // Set PWM duty cycle
    analogWrite(LED_PIN, pwmValue);

    // Hold for 2 seconds
    delay(2000);
  }
}
