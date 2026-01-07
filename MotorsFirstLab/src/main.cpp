// #include <Arduino.h>

// // ===== Encoder Pins =====
// #define ENC_A 39   // GPIO for Encoder Channel A
// #define ENC_B 34   // GPIO for Encoder Channel B

// volatile uint32_t isrHits = 0;
// volatile long encoderCount = 0;

// // ===== ISR for Encoder =====
// // ISR must be fast - no Serial.print or blocking operations!
// void IRAM_ATTR encoderISR() {
//   isrHits++;
//   // Direction logic according to slides: 
//   // If A changes: 
//   // B == A -> forward 
//   // B != A -> backward static long count = 0; 
//   // "static variable" inside ISR (as requirement says) 
//   int a = digitalRead(ENC_A); 
//   int b = digitalRead(ENC_B); 
//   if (a == b) encoderCount++; else encoderCount--; // expose it to loop()
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   pinMode(ENC_A, INPUT_PULLUP);
//   pinMode(ENC_B, INPUT_PULLUP);

//   // Interrupt on any edge change on Channel A
//   attachInterrupt(ENC_A, encoderISR, CHANGE);

//   Serial.println("LAB 1 - Encoder Count Started");
// }

// long readCountAtomic() {
//   noInterrupts();
//   long c = encoderCount;
//   interrupts();
//   return c;
// }

// void loop() {
//   delay(10);  // Small delay to avoid busy-waiting
//   // Always output current encoder count
//   Serial.print("COUNT: ");
//   Serial.println(encoderCount);
//   Serial.print("ISR COUNT: ");
//   Serial.println(isrHits);
//   Serial.println();
// }
