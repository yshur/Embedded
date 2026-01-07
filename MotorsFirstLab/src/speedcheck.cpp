#include <Arduino.h>
#include "esp_timer.h"

// ---------------- GPIO ----------------
#define IN1 25
#define IN2 26

#define ENC_A 34
#define ENC_B 39

// ---------------- ENCODER ----------------
volatile long encoderCount = 0;
volatile bool directionBackward = false;

// ---------------- SPEED ----------------
volatile long lastCount = 0;
volatile float instantSpeed = 0.0; // clicks/sec

// ---------------- TIMER ----------------
esp_timer_handle_t speedTimer;

// =======================================================
// Encoder ISR – counts pulses ONLY
// =======================================================
void IRAM_ATTR encoderISR()
{
    int a = digitalRead(ENC_A);
    int b = digitalRead(ENC_B);

    if ((a ^ b) == 0)
    {
        encoderCount++;
        directionBackward = false;
    }
    else
    {
        encoderCount--;
        directionBackward = true;
    }
}

// =======================================================
// Periodic speed calculation – runs every 100 ms
// =======================================================
void speedTask(void *arg)
{
    long current = encoderCount;
    long delta = current - lastCount;

    instantSpeed = delta / 0.1; // clicks/sec

    Serial.print("Count: ");
    Serial.print(current);
    Serial.print(" | Speed: ");
    Serial.print(instantSpeed);
    Serial.print(" clicks/sec | Dir: ");
    Serial.println(directionBackward ? "Backward" : "Forward");

    lastCount = current;
}

// =======================================================
// SETUP
// =======================================================
void setup()
{
    Serial.begin(115200);

    // Motor pins — keep motor OFF
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    // Encoder pins
    pinMode(ENC_A, INPUT);
    pinMode(ENC_B, INPUT);

    attachInterrupt(digitalPinToInterrupt(ENC_A), encoderISR, CHANGE);

    // ---- Create periodic timer (100 ms) ----
    const esp_timer_create_args_t timer_args = {
        .callback = &speedTask,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "speed_timer"};

    esp_timer_create(&timer_args, &speedTimer);
    esp_timer_start_periodic(speedTimer, 100000); // 100 ms

    Serial.println("Encoder SPEED CHECK (manual rotation)");
}

// =======================================================
// LOOP – nothing needed
// =======================================================
void loop()
{
}
