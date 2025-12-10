/*****************************************************************************
 * | File        : LCDDemo.cpp
 * | Function    : Main demo using OOP LCD library - Spiral drawing demo
 *****************************************************************************/

#include <Arduino.h>
#include "WaveShareDemo.h"

// Global LCD and Touch objects
WaveshareLCD lcd;
LCDTouch touch(lcd);

// ---------- Spiral segment storage ----------
struct Segment {
    int x1, y1, x2, y2;
};

static Segment segments[3000];
static int segCount = 0;

// "touch on spiral" tolerance (pixels)
static const int TOUCH_TOL = 10;  // Increased for easier detection

// Line thickness
static const DotPixel LINE_THICKNESS = DotPixel::PX_4X4;

// Helper clamp
static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void addSegment(int x1, int y1, int x2, int y2) {
    if (segCount < (int)(sizeof(segments)/sizeof(segments[0]))) {
        segments[segCount++] = {x1, y1, x2, y2};
    }
}

static bool pointNearSegment(int tx, int ty, const Segment& s) {
    if (s.y1 == s.y2) {
        int y = s.y1;
        int xmin = min(s.x1, s.x2);
        int xmax = max(s.x1, s.x2);
        return (abs(ty - y) <= TOUCH_TOL) &&
               (tx >= xmin - TOUCH_TOL) && (tx <= xmax + TOUCH_TOL);
    } else if (s.x1 == s.x2) {
        int x = s.x1;
        int ymin = min(s.y1, s.y2);
        int ymax = max(s.y1, s.y2);
        return (abs(tx - x) <= TOUCH_TOL) &&
               (ty >= ymin - TOUCH_TOL) && (ty <= ymax + TOUCH_TOL);
    }
    return false;
}

static bool isTouchOnSpiral(int tx, int ty) {
    for (int i = 0; i < segCount; i++) {
        if (pointNearSegment(tx, ty, segments[i])) return true;
    }
    return false;
}

static void drawEdge(int x1, int y1, int x2, int y2, COLOR color) {
    lcd.drawLine(x1, y1, x2, y2, color, LineStyle::SOLID, LINE_THICKNESS);
    addSegment(x1, y1, x2, y2);
    delay(200);
}

// Returns:
//   true  = finished filling the screen
//   false = interrupted by touch ON the spiral
static bool drawSpiralGrow(COLOR color) {
    segCount = 0;

    int W = lcd.getWidth();
    int H = lcd.getHeight();

    int cx = W / 2;
    int cy = H / 2;

    int left = cx, right = cx, top = cy, bottom = cy;

    // Smaller spacing between lines
    int step = 6;
    int stepInc = 2;

    while (!(left <= 0 && right >= W-1 && top <= 0 && bottom >= H-1)) {
        // right
        right = clampi(right + step, 0, W-1);
        drawEdge(left, top, right, top, color);

        if (touch.scan()) {
            int tx = touch.getX();
            int ty = touch.getY();
            bool onSpiral = isTouchOnSpiral(tx, ty);
            Serial.printf("Touch detected at (%d, %d) - on spiral: %s\n",
                          tx, ty, onSpiral ? "YES" : "NO");
            if (onSpiral) {
                Serial.println(">>> RESETTING SPIRAL <<<");
                return false;
            }
        }

        // bottom
        bottom = clampi(bottom + step, 0, H-1);
        drawEdge(right, top, right, bottom, color);

        if (touch.scan()) {
            int tx = touch.getX();
            int ty = touch.getY();
            bool onSpiral = isTouchOnSpiral(tx, ty);
            Serial.printf("Touch detected at (%d, %d) - on spiral: %s\n",
                          tx, ty, onSpiral ? "YES" : "NO");
            if (onSpiral) {
                Serial.println(">>> RESETTING SPIRAL <<<");
                return false;
            }
        }

        // left
        left = clampi(left - step, 0, W-1);
        drawEdge(right, bottom, left, bottom, color);

        if (touch.scan()) {
            int tx = touch.getX();
            int ty = touch.getY();
            bool onSpiral = isTouchOnSpiral(tx, ty);
            Serial.printf("Touch detected at (%d, %d) - on spiral: %s\n",
                          tx, ty, onSpiral ? "YES" : "NO");
            if (onSpiral) {
                Serial.println(">>> RESETTING SPIRAL <<<");
                return false;
            }
        }

        // top
        top = clampi(top - step, 0, H-1);
        drawEdge(left, bottom, left, top, color);

        if (touch.scan()) {
            int tx = touch.getX();
            int ty = touch.getY();
            bool onSpiral = isTouchOnSpiral(tx, ty);
            Serial.printf("Touch detected at (%d, %d) - on spiral: %s\n",
                          tx, ty, onSpiral ? "YES" : "NO");
            if (onSpiral) {
                Serial.println(">>> RESETTING SPIRAL <<<");
                return false;
            }
        }

        step += stepInc;
    }

    return true;
}

static void drawSpiralShrink(COLOR bgColor) {
    for (int i = segCount - 1; i >= 0; i--) {
        Segment s = segments[i];
        lcd.drawLine(s.x1, s.y1, s.x2, s.y2, bgColor, LineStyle::SOLID, LINE_THICKNESS);
        delay(200);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);  // Wait for Serial to initialize
    Serial.println("=== WaveShareLCD Spiral Demo ===");
    Serial.println("Touch the spiral to reset it!");

    // Initialize LCD with default scan direction and backlight
    lcd.begin(SCAN_DIR_DEFAULT, 200);
    lcd.clear(Colors::WHITE);
    Serial.println("LCD initialized");

    // Initialize touch panel
    touch.begin();
    Serial.println("Touch initialized");
    Serial.println("Starting spiral...");
}

void loop() {
    lcd.clear(Colors::WHITE);

    // Grow spiral. If touched ON spiral -> return false.
    bool finishedNormally = drawSpiralGrow(Colors::BLUE);

    if (finishedNormally) {
        // When spiral fills the screen -> shrink instead of reset
        drawSpiralShrink(Colors::WHITE);
        delay(300);
    } else {
        // When touched ON spiral -> reset immediately (no shrink)
        lcd.clear(Colors::WHITE);
        delay(200);
    }

    // Start over (grow again)
}
