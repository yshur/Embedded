#include <Arduino.h>

// GPIO pin numbers on the ESP32
// LED_PIN = GPIO13  → controls both the onboard blue LED and your external LED
// BTN_PIN = GPIO0   → the built-in button on the ESP32 board
#define LED_ONBOARD 13
#define LED_EXTERNAL 12
#define BTN_PIN 0

void setup() {
  // Configure the LED pin as OUTPUT:
  // This allows the ESP32 to drive voltage out (HIGH) or pull it to GND (LOW)
  pinMode(LED_ONBOARD, OUTPUT);
  pinMode(LED_EXTERNAL, OUTPUT);

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
  if (btnState == LOW) {
    digitalWrite(LED_ONBOARD, HIGH);  // turn LED on
    digitalWrite(LED_EXTERNAL, HIGH);
    delay(500);                       // wait 0.5 second

    digitalWrite(LED_EXTERNAL, LOW);
    delay(500);

    digitalWrite(LED_ONBOARD, LOW);
    digitalWrite(LED_EXTERNAL, HIGH);
    delay(500);

    digitalWrite(LED_EXTERNAL, LOW);
    delay(500);
  } else {
    digitalWrite(LED_ONBOARD, LOW);  // ensure LED stays off
    digitalWrite(LED_EXTERNAL, LOW);
  }
}
