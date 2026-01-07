#include <Arduino.h>

// ===== Encoder Pins =====
#define ENC_A 39   // GPIO for Encoder Channel A
#define ENC_B 34   // GPIO for Encoder Channel B

volatile uint32_t isrHits = 0;
volatile long encoderCount = 0;

// ===== ISR for Encoder =====
// ISR must be fast - no Serial.print or blocking operations!
void IRAM_ATTR encoderISR() {
  isrHits++;
  encoderCount--;          // expose to loop()
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);

  // Interrupt on any edge change on Channel A
  attachInterrupt(ENC_A, encoderISR, CHANGE);

  Serial.println("LAB 1 - Encoder Count Started");
}

void loop() {
  delay(10);  // Small delay to avoid busy-waiting
  // Always output current encoder count
  Serial.print("COUNT: ");
  Serial.println(encoderCount);
  Serial.print("ISR COUNT: ");
  Serial.println(isrHits);
  Serial.println();
}
