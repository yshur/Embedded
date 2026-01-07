// #include <Arduino.h>

// #define ENC_A 34
// #define ENC_B 39

// volatile long encoderCount = 0;
// volatile bool directionBackward = false;

// void IRAM_ATTR encoderISR()
// {
//   int a = digitalRead(ENC_A);
//   int b = digitalRead(ENC_B);

//   // XOR direction logic
//   int dir = a ^ b;

//   if (dir == 0)
//   {
//     encoderCount++; // Forward
//     directionBackward = false;
//   }
//   else
//   {
//     encoderCount--; // Backward
//     directionBackward = true;
//   }
// }

// void setup()
// {
//   Serial.begin(115200);

//   pinMode(ENC_A, INPUT);
//   pinMode(ENC_B, INPUT);

//   attachInterrupt(digitalPinToInterrupt(ENC_A), encoderISR, CHANGE);

//   Serial.println("Encoder XOR direction test started");
// }

// void loop()
// {
//   static long lastCount = 0;

//   if (encoderCount != lastCount)
//   {
//     Serial.print("Count: ");
//     Serial.print(encoderCount);
//     Serial.print(" | Direction: ");
//     Serial.println(directionBackward ? "Backward" : "Forward");

//     lastCount = encoderCount;
//   }

//   delay(10);
// }
