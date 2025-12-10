/*****************************************************************************
 * | File        : LCDDemo.cpp
 * | Function    : Main demo using OOP LCD library
 *****************************************************************************/

#include <Arduino.h>
#include "WaveShareDemo.h"

// Global LCD and Touch objects
WaveshareLCD lcd;
LCDTouch touch(lcd);

void setup()
{
    Serial.begin(115200);

    Serial.println("3.5inch TFT Touch Shield LCD Show...");
    Serial.println("LCD Init...");

    // Initialize LCD with default scan direction and backlight
    lcd.begin(SCAN_DIR_DEFAULT, 200);

    Serial.println("LCD_Clear...");
    lcd.clear(LCD_BACKGROUND);

    Serial.println("LCD_Show...");
    GUI_Show(lcd);

    // Initialize touch panel
    touch.begin();

    // Draw button to move on to time display
    lcd.drawRectangle(200, 170, 360, 170 + 35, Colors::YELLOW, DrawFill::FULL, DotPixel::PX_1X1);
    lcd.drawString(205, 180, "Touch for Time", &Font16, Colors::YELLOW, Colors::BLUE);

    Serial.println("Waiting for touch... (IRQ pin should go LOW when touched)");
    Serial.println("Testing direct SPI read from touch controller...");

    // Direct SPI test - read from XPT2046 regardless of IRQ
    auto testTouchSPI = []() {
        uint16_t rawX = 0, rawY = 0;

        SPI.endTransaction();
        SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

        digitalWrite(4, LOW);  // TP_CS low

        // Read X (command 0xD0)
        SPI.transfer(0xD0);
        delayMicroseconds(200);
        rawX = SPI.transfer(0x00);
        rawX <<= 8;
        rawX |= SPI.transfer(0x00);
        rawX >>= 3;

        // Read Y (command 0x90)
        SPI.transfer(0x90);
        delayMicroseconds(200);
        rawY = SPI.transfer(0x00);
        rawY <<= 8;
        rawY |= SPI.transfer(0x00);
        rawY >>= 3;

        digitalWrite(4, HIGH);  // TP_CS high

        SPI.endTransaction();
        SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

        Serial.printf("  Direct SPI: rawX=%d, rawY=%d (touch=%s)\n",
                      rawX, rawY,
                      (rawX > 100 && rawX < 4000 && rawY > 100 && rawY < 4000) ? "YES" : "no");
    };

    int loopCount = 0;
    while (1) {
        // Print IRQ state and try direct SPI read every second
        if (loopCount % 10 == 0) {
            Serial.printf("IRQ pin (GPIO 16) state: %s\n",
                          touch.isTouching() ? "LOW (touched)" : "HIGH (not touched)");
            testTouchSPI();
        }
        loopCount++;

        if (touch.scan()) {
            POINT x = touch.getX();
            POINT y = touch.getY();
            Serial.printf("Touch detected: X=%d, Y=%d (raw: %d, %d) status=0x%02X\n",
                          x, y, touch.getRawX(), touch.getRawY(),
                          touch.isPressed() ? 0x80 : 0x00);

            if (touch.wasJustPressed()) {
                touch.clearPressedFlag();
                Serial.printf("  -> Button check: x>200=%d, x<360=%d, y>170=%d, y<205=%d\n",
                              x > 200, x < 360, y > 170, y < 170 + 35);
                if (x > 200 && x < 360 && y > 170 && y < 170 + 35) {
                    Serial.println("  -> BUTTON PRESSED! Moving to next screen...");
                    break;
                }
            }
        }
        delay(100);
    }

    lcd.drawRectangle(200, 170, 360, 170 + 35, Colors::WHITE, DrawFill::FULL, DotPixel::PX_1X1);

    // Prepare button to move on to Touch-Draw demonstration in loop()
    lcd.drawRectangle(360, 170, 444, 170 + 35, Colors::BLUE, DrawFill::FULL, DotPixel::PX_1X1);
    lcd.drawString(362, 176, "Next=>", &Font20, Colors::BLUE, Colors::WHITE);
}

void loop()
{
    DevTime devTime;
    devTime.hour = 16;
    devTime.min = 0;
    devTime.sec = 0;

    for (;;) {
        devTime.sec++;
        // Demonstrate updating Time Display
        GUI_Showtime(lcd, 200, 170, 327, 170 + 47, &devTime, Colors::RED);
        delay(1000); // Analog clock 1s
        if (devTime.sec == 60)
            devTime.sec = 0;

        touch.scan();
        if (touch.wasJustPressed()) {
            touch.clearPressedFlag();
            POINT x = touch.getX();
            POINT y = touch.getY();
            if (x > 360 && x < 444 && y > 170 && y < 170 + 35)
                break;
        }
    }

    // Demonstrate Touch Drawing
    TP_Dialog(lcd);
    for (;;) {
        TP_DrawBoard(lcd, touch);
    }
}
