// #include <Arduino.h>
// #include "esp_timer.h"

// // ---------------- GPIO ----------------
// #define IN1 25
// #define IN2 26

// #define ENC_A 34
// #define ENC_B 39

// // ---------------- ENCODER ----------------
// volatile long encoderCount = 0;
// volatile bool directionBackward = false;

// // ---------------- SPEED ----------------
// volatile long lastCount = 0;
// volatile float instantSpeed = 0.0;

// // ---------------- TIMER ----------------
// esp_timer_handle_t speedTimer;

// // ---------------- STATE ----------------
// unsigned long phaseStartTime = 0;
// int phase = 0; // 0=FWD, 1=REV, 2=STOP

// // averages
// long forwardCounts = 0;
// long backwardCounts = 0;

// // =======================================================
// // Encoder ISR – counts pulses ONLY
// // =======================================================
// void IRAM_ATTR encoderISR()
// {
//   int a = digitalRead(ENC_A);
//   int b = digitalRead(ENC_B);

//   if ((a ^ b) == 0)
//   {
//     encoderCount++;
//     directionBackward = false;
//   }
//   else
//   {
//     encoderCount--;
//     directionBackward = true;
//   }
// }

// // =======================================================
// // Periodic speed task – every 100 ms
// // =======================================================
// void speedTask(void *arg)
// {
//   long current = encoderCount;
//   long delta = current - lastCount;

//   instantSpeed = delta / 0.1; // clicks/sec

//   Serial.print("Count: ");
//   Serial.print(current);
//   Serial.print(" | Speed: ");
//   Serial.print(instantSpeed);
//   Serial.print(" clicks/sec | Dir: ");
//   Serial.println(directionBackward ? "Backward" : "Forward");

//   lastCount = current;
// }

// // ---------------- MOTOR CONTROL ----------------
// void motorForward()
// {
//   digitalWrite(IN1, HIGH);
//   digitalWrite(IN2, LOW);
// }

// void motorBackward()
// {
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, HIGH);
// }

// void motorStop()
// {
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, LOW);
// }

// // =======================================================
// // SETUP
// // =======================================================
// void setup()
// {
//   Serial.begin(115200);

//   pinMode(IN1, OUTPUT);
//   pinMode(IN2, OUTPUT);

//   pinMode(ENC_A, INPUT);
//   pinMode(ENC_B, INPUT);

//   attachInterrupt(digitalPinToInterrupt(ENC_A), encoderISR, CHANGE);

//   // ---- Periodic timer setup ----
//   const esp_timer_create_args_t periodic_timer_args = {
//       .callback = &speedTask,
//       .arg = NULL,
//       .dispatch_method = ESP_TIMER_TASK,
//       .name = "speed_timer"};

//   esp_timer_create(&periodic_timer_args, &speedTimer);
//   esp_timer_start_periodic(speedTimer, 100000); // 100 ms

//   // ---- Start FORWARD ----
//   Serial.println("Motor FORWARD (CW)");
//   encoderCount = 0;
//   lastCount = 0;
//   motorForward();

//   phaseStartTime = millis();
//   phase = 0;
// }

// // =======================================================
// // LOOP – non-blocking motor sequencing
// // =======================================================
// void loop()
// {
//   unsigned long now = millis();

//   // After 10 seconds → switch direction
//   if (phase == 0 && now - phaseStartTime >= 10000)
//   {
//     motorStop();
//     forwardCounts = encoderCount;

//     delay(500);

//     Serial.println("Motor BACKWARD (CCW)");
//     encoderCount = 0;
//     lastCount = 0;
//     motorBackward();

//     phaseStartTime = now;
//     phase = 1;
//   }

//   // After another 10 seconds → stop
//   else if (phase == 1 && now - phaseStartTime >= 10000)
//   {
//     motorStop();
//     backwardCounts = abs(encoderCount);

//     esp_timer_stop(speedTimer);

//     Serial.println("\n=== AVERAGE SPEEDS ===");

//     Serial.print("Forward avg speed: ");
//     Serial.print(forwardCounts / 10.0);
//     Serial.println(" clicks/sec");

//     Serial.print("Backward avg speed: ");
//     Serial.print(backwardCounts / 10.0);
//     Serial.println(" clicks/sec");

//     phase = 2; // done
//   }
// }
