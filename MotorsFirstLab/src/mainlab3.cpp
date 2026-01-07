// #include <Arduino.h>

// /* =====================================================
//  * GPIO DEFINITIONS
//  * ===================================================== */

// // Motor driver
// #define IN1 25
// #define IN2 26
// #define PWM_PIN 21 // EN pin (A-EN jumper removed)

// // Encoder
// #define ENC_A 39
// #define ENC_B 34

// /* =====================================================
//  * PWM (LEDC)
//  * ===================================================== */
// #define PWM_CH 0
// #define PWM_FREQ 20000 // 20 kHz
// #define PWM_RES 8      // 8-bit → 0–255

// /* =====================================================
//  * ENCODER STATE
//  * ===================================================== */
// volatile long encoderCount = 0;

// /* =====================================================
//  * LAB 3 CONSTANTS
//  * ===================================================== */
// const int kickStartTimeMs = 10; // already measured
// const int pwmStep = 5;

// /* =====================================================
//  * MOTOR CONTROL
//  * ===================================================== */
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

// /* =====================================================
//  * ENCODER ISR
//  * ===================================================== */
// void IRAM_ATTR encoderISR()
// {
//     int a = digitalRead(ENC_A);
//     int b = digitalRead(ENC_B);

//     if ((a ^ b) == 0)
//         encoderCount++;
//     else
//         encoderCount--;
// }

// /* =====================================================
//  * LAB 3 – PART 2
//  * Find minimum sustaining PWM
//  * ===================================================== */
// int findMinimumSustainingPWM()
// {
//     const int kickStartTimeMs = 10;    // already measured
//     const int pwmStart = 200;          // start from 200
//     const int pwmStep = 5;             // decrement step
//     const int OBSERVE_SECONDS = 10;    // observe for 10 seconds
//     const int MAX_DROP = 50;           // allowed drop per second
//     const int MIN_PULSES_PER_SEC = 10; // detect stall

//     Serial.println("\nLAB 3 – Finding minimum sustaining PWM");
//     Serial.print("Kick-start time = ");
//     Serial.print(kickStartTimeMs);
//     Serial.println(" ms\n");

//     for (int pwm = pwmStart; pwm >= 0; pwm -= pwmStep)
//     {
//         encoderCount = 0;

//         // ---------- Kick-start ----------
//         motorForward();
//         ledcWrite(PWM_CH, 255);
//         delay(kickStartTimeMs);

//         // ---------- Drop to test PWM ----------
//         ledcWrite(PWM_CH, pwm);

//         bool stable = true;
//         long prevMag = -1;

//         // ---------- Observe for 10 seconds ----------
//         for (int sec = 1; sec <= OBSERVE_SECONDS; sec++)
//         {
//             long startCount = encoderCount;
//             delay(1000);
//             long pulses = encoderCount - startCount;

//             long mag = labs(pulses);

//             Serial.print("PWM ");
//             Serial.print(pwm);
//             Serial.print(" | t=");
//             Serial.print(sec);
//             Serial.print("s | pulses=");
//             Serial.println(pulses);

//             // Must still be rotating
//             if (mag < MIN_PULSES_PER_SEC)
//                 stable = false;

//             // Speed drop check (allow max drop of 50)
//             if (prevMag != -1 && (prevMag - mag) > MAX_DROP)
//                 stable = false;

//             prevMag = mag;
//         }

//         motorStop();
//         delay(500);

//         if (!stable)
//         {
//             Serial.print("\n❌ Unstable at PWM ");
//             Serial.println(pwm);
//             Serial.print("✅ Minimum sustaining PWM = ");
//             Serial.println(pwm + pwmStep);
//             return pwm + pwmStep;
//         }

//         Serial.print("✅ Stable at PWM ");
//         Serial.println(pwm);
//     }

//     return 0; // fallback
// }

// /* =====================================================
//  * SETUP
//  * ===================================================== */
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

//     // ---------- LAB 3 PART 2 ----------
//     int minPWM = findMinimumSustainingPWM();

//     Serial.println("\n=== LAB 3 RESULT ===");
//     Serial.print("Minimum sustaining PWM = ");
//     Serial.println(minPWM);
// }

// /* =====================================================
//  * LOOP – not used
//  * ===================================================== */
// void loop()
// {
// }
