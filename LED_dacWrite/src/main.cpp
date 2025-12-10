#include <Arduino.h>

// DAC LED on GPIO25 (or GPIO26)
// dacWrite only works on pins 25 and 26
#define LED_PIN 25

void setup() {
  // No pinMode needed for DAC output
}

void loop() {
  // Loop from 0% to 100% in 10% steps
  // dacWrite uses 0-255 range → 0V to 3.2V
  for (int percent = 0; percent <= 100; percent += 10) {
    // Convert percentage to 0-255 range
    int dacValue = (percent * 255) / 100;

    // Set DAC output voltage
    dacWrite(LED_PIN, dacValue);

    // Hold for 2 seconds
    delay(2000);
  }
}
