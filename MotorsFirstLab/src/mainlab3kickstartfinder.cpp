// #include <Arduino.h>

// /* ===================== GPIO ===================== */
// #define IN1 25
// #define IN2 26
// #define PWM_PIN 21 // EN pin (A-EN jumper removed)

// #define ENC_A 34
// #define ENC_B 39

// /* ===================== PWM ===================== */
// #define PWM_CH 0
// #define PWM_FREQ 20000 // 20 kHz
// #define PWM_RES 8      // 8-bit (0–255)

// /* ===================== ENCODER ===================== */
// volatile long encoderCount = 0;

// /* ===================== STORED RESULT ===================== */
// int kickStartTimeMs = -1;

// /* =========================================================
//  * Encoder ISR – counts pulses ONLY
//  * ========================================================= */
// void IRAM_ATTR encoderISR()
// {
//     int a = digitalRead(ENC_A);
//     int b = digitalRead(ENC_B);

//     if ((a ^ b) == 0)
//         encoderCount++;
//     else
//         encoderCount--;
// }

// /* ===================== MOTOR CONTROL ===================== */
// void motorForward()
// {
//     digitalWrite(IN1, HIGH);
//     digitalWrite(IN2, LOW);
// }

// void motorStop()
// {
//     digitalWrite(IN1, LOW);
//     digitalWrite(IN2, LOW);
//     ledcWrite(PWM_CH, 0);
// }

// /* =========================================================
//  * Binary-search kick-start time (10–30 ms)
//  * ========================================================= */
// int findKickStartTime()
// {
//     int low = 5;
//     int high = 30;
//     int result = -1;

//     Serial.println("\nFinding kick-start time (binary search)");

//     while (low <= high)
//     {
//         int mid = (low + high) / 2;

//         encoderCount = 0;

//         motorForward();
//         ledcWrite(PWM_CH, 255); // full PWM
//         delay(mid);
//         motorStop();

//         delay(100); // allow encoder to settle

//         Serial.print("Test ");
//         Serial.print(mid);
//         Serial.print(" ms → encoderCount = ");
//         Serial.println(encoderCount);

//         if (abs(encoderCount) > 0)
//         {
//             result = mid;   // motor started
//             high = mid - 1; // try smaller time
//         }
//         else
//         {
//             low = mid + 1; // need more time
//         }
//     }

//     return result;
// }

// /* =========================================================
//  * SETUP – calibration ONLY
//  * ========================================================= */
// void setup()
// {
//     Serial.begin(115200);

//     // Motor pins
//     pinMode(IN1, OUTPUT);
//     pinMode(IN2, OUTPUT);

//     // Encoder pins
//     pinMode(ENC_A, INPUT);
//     pinMode(ENC_B, INPUT);
//     attachInterrupt(digitalPinToInterrupt(ENC_A), encoderISR, CHANGE);

//     // PWM setup
//     ledcSetup(PWM_CH, PWM_FREQ, PWM_RES);
//     ledcAttachPin(PWM_PIN, PWM_CH);

//     motorStop();
//     delay(1000);

//     // ---------- KICK-START CALIBRATION ----------
//     kickStartTimeMs = findKickStartTime();

//     Serial.println("\n=== KICK-START RESULT ===");
//     Serial.print("Minimum kick-start time = ");
//     Serial.print(kickStartTimeMs);
//     Serial.println(" ms");

//     Serial.println("Value saved for LAB 3 usage");
// }

// /* =========================================================
//  * LOOP – not used
//  * ========================================================= */
// void loop()
// {
// }
