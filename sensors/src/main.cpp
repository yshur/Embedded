#include <Arduino.h>
/*************************************************
 * LAB 2 – Sensors & Actuators (ESP32 Thing Plus)
 *************************************************/

// ----- Pins -----
int grayPin  = 34;   // Grayscale Sensor (Analog)
int tiltPin  = 32;   // Tilt Sensor (Digital)

int buzzer   = 25;   // Buzzer (PWM)
int ledPin   = 13;   // Built-in LED (optional)

// PWM Channels
#define BUZ_CHANNEL  0

// void playMelody(int melodyNum = 1) {
//     int melody[] = { 523, 587, 659, 784 };
//     int melody2[] = { 784, 659, 523, 659 };
//     int duration[] = { 200, 200, 200, 400 };
//     int duration2[] = { 150, 150, 300, 400 };

//     // int* melody = (melodyNum == 1) ? melody1 : melody2;
//     // int* duration = (melodyNum == 1) ? duration1 : duration2;

//     for (int i = 0; i < 4; i++) {
//         ledcWriteTone(BUZ_CHANNEL, melody[i]);
//         ledcWrite(BUZ_CHANNEL, 150);   // 60% duty
//         delay(duration[i]);
//         ledcWrite(BUZ_CHANNEL, 0);
//         delay(50);
//     }
// }

void setup() {
    Serial.begin(115200);

    // Inputs
    pinMode(grayPin, INPUT);
    pinMode(tiltPin, INPUT);

    // Built-in LED
    pinMode(ledPin, OUTPUT);

    // PWM for buzzer
    ledcSetup(BUZ_CHANNEL, 2000, 8);
    ledcAttachPin(buzzer, BUZ_CHANNEL);
    ledcWrite(BUZ_CHANNEL, 0);

    Serial.println("=== LAB2 System Started ===");
}

void loop() {
    // ledcWrite(BUZ_CHANNEL, 180); // 0..255
    // delay(500);
    // ledcWrite(BUZ_CHANNEL, 0);
    // delay(2000);
    // return;

    // ----- Grayscale Sensor -----
    int grayValue = analogRead(grayPin);
    Serial.println(grayValue);

    int volume = map(grayValue, 0, 4095, 50, 255);
    Serial.println(volume);
    // ledcWrite(BUZ_CHANNEL, volume);
    delay(200);
    // return;

    // ----- Tilt Sensor -----
    int tiltState = digitalRead(tiltPin);
    if (tiltState == HIGH) {
        Serial.println("Tilt detected!");
        // ledcWrite(BUZ_CHANNEL, 120);
        delay(80);
        // ledcWrite(BUZ_CHANNEL, 0);
    }

    // Debug print
    Serial.print("Gray=");
    Serial.print(grayValue);
    Serial.print(" | Tilt=");
    Serial.println(tiltState);

    delay(100);
}
