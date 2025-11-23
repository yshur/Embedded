#include <Arduino.h>

// GPIO pin numbers on the ESP32
// LED_PIN = GPIO13  → controls both the onboard blue LED and your external LED
// BTN_PIN = GPIO0   → the built-in button on the ESP32 board
#define LED_PIN 13
#define BTN_PIN 0

void setup() {
  // Configure the LED pin as OUTPUT:
  // This allows the ESP32 to drive voltage out (HIGH) or pull it to GND (LOW)
  pinMode(LED_PIN, OUTPUT);

  // Configure the button pin as INPUT:
  // The button is wired on the board such that:
  //   - When NOT pressed → the pin reads HIGH
  //   - When pressed     → the pin is pulled to LOW
  pinMode(BTN_PIN, INPUT);
}

void loop() {
  // Read the current state of the button:
  // digitalRead returns HIGH or LOW depending on the voltage on the pin
  int btnState = digitalRead(BTN_PIN);

  // If the button is NOT pressed (btnState == HIGH):
  //    Blink the LED every 1 second
  // If the button IS pressed (btnState == LOW):
  //    Turn the LED off
  if (btnState == HIGH) {
    digitalWrite(LED_PIN, HIGH); // turn LED on
    delay(1000);                 // wait 1 second
    digitalWrite(LED_PIN, LOW);  // turn LED off
    delay(1000);                 // wait 1 second
  } else {
    digitalWrite(LED_PIN, LOW);  // ensure LED stays off
  }
}
