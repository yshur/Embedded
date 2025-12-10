#include <Arduino.h>
#include "WaveShareDemo.h"

// ---------- Spiral segment storage ----------
struct Segment {
  int x1, y1, x2, y2;
};

static Segment segments[3000];
static int segCount = 0;

// "touch on spiral" tolerance (pixels)
static const int TOUCH_TOL = 4;

// Line thickness: make it thicker
static const DOT_PIXEL LINE_THICKNESS = DOT_PIXEL_4X4;

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
  GUI_DrawLine(x1, y1, x2, y2, color, LINE_SOLID, LINE_THICKNESS);
  addSegment(x1, y1, x2, y2);
  delay(200);
}

// Returns:
//   true  = finished filling the screen
//   false = interrupted by touch ON the spiral
static bool drawSpiralGrow(COLOR color) {
  segCount = 0;

  int W = sLCD_DIS.LCD_Dis_Column;
  int H = sLCD_DIS.LCD_Dis_Page;

  int cx = W / 2;
  int cy = H / 2;

  int left = cx, right = cx, top = cy, bottom = cy;

  // Smaller spacing between lines:
  // start small and grow slowly
  int step = 6;       // was 10
  int stepInc = 2;    // was 4

  while (!(left <= 0 && right >= W-1 && top <= 0 && bottom >= H-1)) {
    // right
    right = clampi(right + step, 0, W-1);
    drawEdge(left, top, right, top, color);

    TP_Scan(0);
    if (sTP_DEV.chStatus & TP_PRESSED) {
      sTP_DEV.chStatus &= ~TP_PRESSED;
      if (isTouchOnSpiral(sTP_Draw.Xpoint, sTP_Draw.Ypoint)) return false;
    }

    // bottom
    bottom = clampi(bottom + step, 0, H-1);
    drawEdge(right, top, right, bottom, color);

    TP_Scan(0);
    if (sTP_DEV.chStatus & TP_PRESSED) {
      sTP_DEV.chStatus &= ~TP_PRESSED;
      if (isTouchOnSpiral(sTP_Draw.Xpoint, sTP_Draw.Ypoint)) return false;
    }

    // left
    left = clampi(left - step, 0, W-1);
    drawEdge(right, bottom, left, bottom, color);

    TP_Scan(0);
    if (sTP_DEV.chStatus & TP_PRESSED) {
      sTP_DEV.chStatus &= ~TP_PRESSED;
      if (isTouchOnSpiral(sTP_Draw.Xpoint, sTP_Draw.Ypoint)) return false;
    }

    // top
    top = clampi(top - step, 0, H-1);
    drawEdge(left, bottom, left, top, color);

    TP_Scan(0);
    if (sTP_DEV.chStatus & TP_PRESSED) {
      sTP_DEV.chStatus &= ~TP_PRESSED;
      if (isTouchOnSpiral(sTP_Draw.Xpoint, sTP_Draw.Ypoint)) return false;
    }

    step += stepInc;
  }

  return true;
}

static void drawSpiralShrink(COLOR bgColor) {
  for (int i = segCount - 1; i >= 0; i--) {
    Segment s = segments[i];
    GUI_DrawLine(s.x1, s.y1, s.x2, s.y2, bgColor, LINE_SOLID, LINE_THICKNESS);
    delay(200);
  }
}

void setup() {
  Wvshr_Init();
  LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;
  LCD_Init(Lcd_ScanDir, 200);
  LCD_Clear(WHITE);
  TP_Init();
}

void loop() {
  LCD_Clear(WHITE);

  // Grow spiral. If touched ON spiral -> return false.
  bool finishedNormally = drawSpiralGrow(BLUE);

  if (finishedNormally) {
    // NEW behavior:
    // When spiral fills the screen -> shrink instead of reset
    drawSpiralShrink(WHITE);
    delay(300);
  } else {
    // NEW behavior:
    // When touched ON spiral -> reset immediately (no shrink)
    LCD_Clear(WHITE);
    delay(200);
  }

  // Start over (grow again)
}