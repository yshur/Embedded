/*****************************************************************************
 * | File        : WaveShareDemo.cpp
 * | Function    : Demo functions using OOP LCD library
 *****************************************************************************/

#include <Arduino.h>
#include "WaveShareDemo.h"

/******************************************************************************
  function:   According to the display area adaptive display time
******************************************************************************/
void GUI_Showtime(WaveshareLCD& lcd, POINT xStart, POINT yStart,
                  POINT xEnd, POINT yEnd, DevTime* pTime, COLOR color) {
    uint8_t value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    sFONT* font = nullptr;

    POINT dx = (xEnd - xStart) / 7;
    POINT dy = yEnd - yStart;
    font = lcd.getFontForSize(dx, dy);

    if ((pTime->sec % 10) < 10 && (pTime->sec % 10) > 0) {
        lcd.fillArea(xStart + dx * 6, yStart, xEnd, yEnd, Colors::WHITE);
    } else {
        if ((pTime->sec / 10) < 6 && (pTime->sec / 10) > 0) {
            lcd.fillArea(xStart + dx * 5, yStart, xEnd, yEnd, Colors::WHITE);
        } else {
            pTime->min = pTime->min + 1;
            pTime->sec = 0;
            if ((pTime->min % 10) < 10 && (pTime->min % 10) > 0) {
                lcd.fillArea(xStart + dx * 3 + dx / 2, yStart, xEnd, yEnd, Colors::WHITE);
            } else {
                if ((pTime->min / 10) < 6 && (pTime->min / 10) > 0) {
                    lcd.fillArea(xStart + dx * 2 + dx / 2, yStart, xEnd, yEnd, Colors::WHITE);
                } else {
                    pTime->hour = pTime->hour + 1;
                    pTime->min = 0;
                    if ((pTime->hour % 10) < 4 && (pTime->hour % 10) > 0 && pTime->hour < 24) {
                        lcd.fillArea(xStart + dx, yStart, xEnd, yEnd, Colors::WHITE);
                    } else {
                        pTime->hour = 0;
                        pTime->min = 0;
                        pTime->sec = 0;
                        lcd.fillArea(xStart, yStart, xEnd, yEnd, Colors::WHITE);
                    }
                }
            }
        }
    }

    lcd.drawChar(xStart, yStart, value[pTime->hour / 10], font, FONT_BACKGROUND, color);
    lcd.drawChar(xStart + dx, yStart, value[pTime->hour % 10], font, FONT_BACKGROUND, color);
    lcd.drawChar(xStart + dx + dx / 4 + dx / 2, yStart, ':', font, FONT_BACKGROUND, color);
    lcd.drawChar(xStart + dx * 2 + dx / 2, yStart, value[pTime->min / 10], font, FONT_BACKGROUND, color);
    lcd.drawChar(xStart + dx * 3 + dx / 2, yStart, value[pTime->min % 10], font, FONT_BACKGROUND, color);
    lcd.drawChar(xStart + dx * 4 + dx / 2 - dx / 4, yStart, ':', font, FONT_BACKGROUND, color);
    lcd.drawChar(xStart + dx * 5, yStart, value[pTime->sec / 10], font, FONT_BACKGROUND, color);
    lcd.drawChar(xStart + dx * 6, yStart, value[pTime->sec % 10], font, FONT_BACKGROUND, color);
}

/******************************************************************************
  function:   GUI_Show - Demo of drawing capabilities
******************************************************************************/
void GUI_Show(WaveshareLCD& lcd) {
    lcd.clear(Colors::WHITE);

    LENGTH width = lcd.getWidth();
    LENGTH height = lcd.getHeight();

    if (width > height) {
        // Horizontal screen display
        Serial.println("Draw Horizontal Lines");
        lcd.drawLine(0, 10, width, 10, Colors::RED, LineStyle::DOTTED, DotPixel::PX_8X8);
        lcd.drawLine(0, 20, width, 20, Colors::RED, LineStyle::DOTTED, DOT_PIXEL_DEFAULT);
        lcd.drawLine(0, 300, width, 300, Colors::RED, LineStyle::DOTTED, DOT_PIXEL_DEFAULT);
        lcd.drawLine(0, 310, width, 310, Colors::RED, LineStyle::DOTTED, DotPixel::PX_4X4);

        Serial.println("Draw Rectangle");
        lcd.drawRectangle(10, 30, width - 10, height - 30, Colors::BLUE, DrawFill::EMPTY, DotPixel::PX_4X4);
        lcd.drawRectangle(20, 40, width - 20, 60, Colors::BLUE, DrawFill::FULL, DOT_PIXEL_DEFAULT);
        lcd.drawRectangle(20, 65, width - 25, height - 40, Colors::MAGENTA, DrawFill::EMPTY, DotPixel::PX_6X6, LineStyle::DOTTED);

        Serial.println("Draw Olympic Rings");
        uint16_t cx1 = 190, cy1 = 240, cr = 20;
        uint16_t cx2 = cx1 + (2.5 * cr), cy2 = cy1;
        uint16_t cx3 = cx1 + (5 * cr), cy3 = cy1;
        uint16_t cx4 = (cx1 + cx2) / 2, cy4 = cy1 + cr;
        uint16_t cx5 = (cx2 + cx3) / 2, cy5 = cy1 + cr;

        lcd.drawCircle(cx1, cy1, cr, Colors::BLUE, DrawFill::EMPTY, DotPixel::PX_3X3);
        lcd.drawCircle(cx2, cy2, cr, Colors::BLACK, DrawFill::EMPTY, DotPixel::PX_3X3);
        lcd.drawCircle(cx3, cy3, cr, Colors::RED, DrawFill::EMPTY, DotPixel::PX_3X3);
        lcd.drawCircle(cx4, cy4, cr, Colors::YELLOW, DrawFill::EMPTY, DotPixel::PX_3X3);
        lcd.drawCircle(cx5, cy5, cr, Colors::GREEN, DrawFill::EMPTY, DotPixel::PX_3X3);

        Serial.println("Draw Realistic circles");
        lcd.drawCircle(58, 250, 30, Colors::CYAN, DrawFill::FULL, DOT_PIXEL_DEFAULT);
        lcd.drawCircle(width - 55, 250, 30, Colors::CYAN, DrawFill::EMPTY, DotPixel::PX_4X4);

        Serial.println("Display String");
        lcd.drawString(80, 80, "WaveShare Electronic", &Font24, LCD_BACKGROUND, Colors::BLUE);
        lcd.drawString(80, 120, "3.5inch TFTLCD", &Font20, Colors::YELLOW, Colors::BLUE);

        Serial.println("Display Number");
        lcd.drawNumber(80, 150, 1234567890, &Font16, LCD_BACKGROUND, Colors::BLUE);

    } else {
        // Vertical screen display
        Serial.println("Draw Line");
        lcd.drawLine(0, 10, width, 10, Colors::RED, LineStyle::SOLID, DotPixel::PX_2X2);
        lcd.drawLine(0, 20, width, 20, Colors::RED, LineStyle::DOTTED, DOT_PIXEL_DEFAULT);
        lcd.drawLine(0, height - 20, width, height - 20, Colors::RED, LineStyle::DOTTED, DOT_PIXEL_DEFAULT);
        lcd.drawLine(0, height - 10, width, height - 10, Colors::RED, LineStyle::SOLID, DotPixel::PX_2X2);

        Serial.println("Draw Rectangle");
        lcd.drawRectangle(10, 30, width - 10, height - 30, Colors::BLUE, DrawFill::EMPTY, DOT_PIXEL_DEFAULT);
        lcd.drawRectangle(20, 40, width - 20, 60, Colors::BLUE, DrawFill::FULL, DOT_PIXEL_DEFAULT);

        Serial.println("Draw Olympic Rings");
        uint16_t cx1 = 120, cy1 = 300, cr = 20;
        uint16_t cx2 = cx1 + (2.5 * cr), cy2 = cy1;
        uint16_t cx3 = cx1 + (5 * cr), cy3 = cy1;
        uint16_t cx4 = (cx1 + cx2) / 2, cy4 = cy1 + cr;
        uint16_t cx5 = (cx2 + cx3) / 2, cy5 = cy1 + cr;

        lcd.drawCircle(cx1, cy1, cr, Colors::BLUE, DrawFill::EMPTY, DotPixel::PX_2X2);
        lcd.drawCircle(cx2, cy2, cr, Colors::BLACK, DrawFill::EMPTY, DotPixel::PX_2X2);
        lcd.drawCircle(cx3, cy3, cr, Colors::RED, DrawFill::EMPTY, DotPixel::PX_2X2);
        lcd.drawCircle(cx4, cy4, cr, Colors::YELLOW, DrawFill::EMPTY, DotPixel::PX_2X2);
        lcd.drawCircle(cx5, cy5, cr, Colors::GREEN, DrawFill::EMPTY, DotPixel::PX_2X2);

        Serial.println("Draw Realistic circles");
        lcd.drawCircle(50, 400, 30, Colors::CYAN, DrawFill::FULL, DOT_PIXEL_DEFAULT);
        lcd.drawCircle(width - 50, 400, 30, Colors::CYAN, DrawFill::FULL, DOT_PIXEL_DEFAULT);

        Serial.println("Display String");
        lcd.drawString(40, 120, "WaveShare Electronic", &Font24, LCD_BACKGROUND, Colors::BLUE);
        lcd.drawString(40, 180, "3.5inch TFTLCD", &Font20, Colors::RED, Colors::BLUE);

        Serial.println("Display Number");
        lcd.drawNumber(40, 210, 1234567890, &Font16, LCD_BACKGROUND, Colors::BLUE);
    }
}

/******************************************************************************
  function:   Paint the Delete key and paint color choose area
******************************************************************************/
void TP_Dialog(WaveshareLCD& lcd) {
    lcd.clear(LCD_BACKGROUND);
    Serial.println("Drawing...");

    LENGTH width = lcd.getWidth();
    LENGTH height = lcd.getHeight();

    if (width > height) {
        // Horizontal screen display
        lcd.drawString(width - 60, 0, "CLEAR", &Font16, Colors::RED, Colors::BLUE);
        lcd.drawString(width - 120, 0, "AD", &Font24, Colors::RED, Colors::BLUE);

        lcd.drawRectangle(width - 50, 20, width, 70, Colors::BLUE, DrawFill::FULL, DotPixel::PX_1X1);
        lcd.drawRectangle(width - 50, 80, width, 130, Colors::GREEN, DrawFill::FULL, DotPixel::PX_1X1);
        lcd.drawRectangle(width - 50, 140, width, 190, Colors::RED, DrawFill::FULL, DotPixel::PX_1X1);
        lcd.drawRectangle(width - 50, 200, width, 250, Colors::YELLOW, DrawFill::FULL, DotPixel::PX_1X1);
        lcd.drawRectangle(width - 50, 260, width, 310, Colors::BLACK, DrawFill::FULL, DotPixel::PX_1X1);
    } else {
        // Vertical screen display
        lcd.drawString(width - 60, 0, "CLEAR", &Font16, Colors::RED, Colors::BLUE);
        lcd.drawString(width - 120, 0, "AD", &Font24, Colors::RED, Colors::BLUE);

        lcd.drawRectangle(20, 20, 70, 70, Colors::BLUE, DrawFill::FULL, DotPixel::PX_1X1);
        lcd.drawRectangle(80, 20, 130, 70, Colors::GREEN, DrawFill::FULL, DotPixel::PX_1X1);
        lcd.drawRectangle(140, 20, 190, 70, Colors::RED, DrawFill::FULL, DotPixel::PX_1X1);
        lcd.drawRectangle(200, 20, 250, 70, Colors::YELLOW, DrawFill::FULL, DotPixel::PX_1X1);
        lcd.drawRectangle(260, 20, 310, 70, Colors::BLACK, DrawFill::FULL, DotPixel::PX_1X1);
    }
}

/******************************************************************************
  function:   Draw Board - touch paint application
******************************************************************************/
void TP_DrawBoard(WaveshareLCD& lcd, LCDTouch& touch) {
    touch.scan();

    if (!touch.isPressed()) return;

    POINT x = touch.getX();
    POINT y = touch.getY();
    LENGTH width = lcd.getWidth();
    LENGTH height = lcd.getHeight();

    if (x >= width || y >= height) return;

    if (width > height) {
        // Horizontal screen
        if (x > (width - 60) && y < 16) {
            TP_Dialog(lcd);
        } else if (x > (width - 120) && x < (width - 80) && y < 24) {
            touch.calibrate();
            TP_Dialog(lcd);
        } else if (x > (width - 50) && x < width && y > 20 && y < 70) {
            touch.setColor(Colors::BLUE);
        } else if (x > (width - 50) && x < width && y > 80 && y < 130) {
            touch.setColor(Colors::GREEN);
        } else if (x > (width - 50) && x < width && y > 140 && y < 190) {
            touch.setColor(Colors::RED);
        } else if (x > (width - 50) && x < width && y > 200 && y < 250) {
            touch.setColor(Colors::YELLOW);
        } else if (x > (width - 50) && x < width && y > 260 && y < 310) {
            touch.setColor(Colors::BLACK);
        } else {
            lcd.drawPoint(x, y, touch.getColor(), DotPixel::PX_2X2, DotStyle::FILL_RIGHTUP);
        }
    } else {
        // Vertical screen
        if (x > (width - 60) && y < 16) {
            TP_Dialog(lcd);
        } else if (x > (width - 120) && x < (width - 80) && y < 24) {
            touch.calibrate();
            TP_Dialog(lcd);
        } else if (x > 20 && x < 70 && y > 20 && y < 70) {
            touch.setColor(Colors::BLUE);
        } else if (x > 80 && x < 130 && y > 20 && y < 70) {
            touch.setColor(Colors::GREEN);
        } else if (x > 140 && x < 190 && y > 20 && y < 70) {
            touch.setColor(Colors::RED);
        } else if (x > 200 && x < 250 && y > 20 && y < 70) {
            touch.setColor(Colors::YELLOW);
        } else if (x > 260 && x < 310 && y > 20 && y < 70) {
            touch.setColor(Colors::BLACK);
        } else {
            lcd.drawPoint(x, y, touch.getColor(), DotPixel::PX_2X2, DotStyle::FILL_RIGHTUP);
        }
    }
}
