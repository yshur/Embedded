#include <Arduino.h>
#include "WaveShareDemo.h"   // LCD_Driver / LCD_GUI / LCD_Touch
#include "I2Cdev.h"
#include "MPU6050.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  #include "Wire.h"
#endif

// ---------------- MPU6050 ----------------
MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

// Low-pass filter to reduce jitter
static float fax = 0, fay = 0, faz = 0;
static const float ALPHA = 0.85f; // closer to 1 = more smoothing

// Previous positions for selective erasing
static int prevPx = -9999, prevPy = -9999, prevZy = -9999;
static const int MOVE_THRESH = 3;

// Screen dimensions and layout
static int W, H, cx, cy;
static int zx = 60;  // X position for Z axis display
static int xyBoxLeft, xyBoxRight, xyBoxTop, xyBoxBottom;

// Temperature update counter
static unsigned long lastTempUpdate = 0;
static const unsigned long TEMP_UPDATE_INTERVAL = 1000; // Update temp every 1 second

// ---------------- Drawing helpers ----------------
static int clampi(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// Erase arrow by drawing it in background color
void EraseArrowXY(int cx, int cy, int px, int py, COLOR color = WHITE) {
  if (px == -9999) return; // Never drawn before

  int dx = px - cx;
  int dy = py - cy;
  float len = sqrtf((float)dx*dx + (float)dy*dy);

  // Erase main vector
  GUI_DrawLine(cx, cy, px, py, color, LINE_SOLID, DOT_PIXEL_3X3);

  if (len < 1) return;

  float ang = atan2f((float)dy, (float)dx);
  const float headAng = 0.5f;
  const int headLen = 12;

  int hx1 = px - (int)(cosf(ang - headAng) * headLen);
  int hy1 = py - (int)(sinf(ang - headAng) * headLen);
  int hx2 = px - (int)(cosf(ang + headAng) * headLen);
  int hy2 = py - (int)(sinf(ang + headAng) * headLen);

  GUI_DrawLine(px, py, hx1, hy1, color, LINE_SOLID, DOT_PIXEL_2X2);
  GUI_DrawLine(px, py, hx2, hy2, color, LINE_SOLID, DOT_PIXEL_2X2);
}

void DrawArrowXY(int cx, int cy, int px, int py) {
  int dx = px - cx;
  int dy = py - cy;
  float len = sqrtf((float)dx*dx + (float)dy*dy);

  // Main vector
  GUI_DrawLine(cx, cy, px, py, RED, LINE_SOLID, DOT_PIXEL_3X3);

  if (len < 1) return;

  float ang = atan2f((float)dy, (float)dx);
  const float headAng = 0.5f;
  const int headLen = 12;

  int hx1 = px - (int)(cosf(ang - headAng) * headLen);
  int hy1 = py - (int)(sinf(ang - headAng) * headLen);
  int hx2 = px - (int)(cosf(ang + headAng) * headLen);
  int hy2 = py - (int)(sinf(ang + headAng) * headLen);

  GUI_DrawLine(px, py, hx1, hy1, RED, LINE_SOLID, DOT_PIXEL_2X2);
  GUI_DrawLine(px, py, hx2, hy2, RED, LINE_SOLID, DOT_PIXEL_2X2);
}

void EraseArrowZ(int zx, int cy, int zy, COLOR color = WHITE) {
  if (zy == -9999) return;

  GUI_DrawLine(zx, cy, zx, zy, color, LINE_SOLID, DOT_PIXEL_3X3);

  int headLen = 10;
  int headWid = 6;

  if (zy < cy) {
    GUI_DrawLine(zx, zy, zx - headWid, zy + headLen, color, LINE_SOLID, DOT_PIXEL_2X2);
    GUI_DrawLine(zx, zy, zx + headWid, zy + headLen, color, LINE_SOLID, DOT_PIXEL_2X2);
  } else {
    GUI_DrawLine(zx, zy, zx - headWid, zy - headLen, color, LINE_SOLID, DOT_PIXEL_2X2);
    GUI_DrawLine(zx, zy, zx + headWid, zy - headLen, color, LINE_SOLID, DOT_PIXEL_2X2);
  }
}

void DrawArrowZ(int zx, int cy, int zy) {
  GUI_DrawLine(zx, cy, zx, zy, RED, LINE_SOLID, DOT_PIXEL_3X3);

  int headLen = 10;
  int headWid = 6;

  if (zy < cy) { // up
    GUI_DrawLine(zx, zy, zx - headWid, zy + headLen, RED, LINE_SOLID, DOT_PIXEL_2X2);
    GUI_DrawLine(zx, zy, zx + headWid, zy + headLen, RED, LINE_SOLID, DOT_PIXEL_2X2);
  } else {       // down
    GUI_DrawLine(zx, zy, zx - headWid, zy - headLen, RED, LINE_SOLID, DOT_PIXEL_2X2);
    GUI_DrawLine(zx, zy, zx + headWid, zy - headLen, RED, LINE_SOLID, DOT_PIXEL_2X2);
  }
}

// Draw static UI elements (axes, labels, borders)
void DrawStaticUI() {
  // Draw vertical separator between Z axis and XY plane
  GUI_DrawLine(zx + 40, 0, zx + 40, H, GRAY, LINE_SOLID, DOT_PIXEL_1X1);

  // XY plane axes
  GUI_DrawLine(cx, xyBoxTop, cx, xyBoxBottom, BLACK, LINE_SOLID, DOT_PIXEL_1X1);  // Y axis
  GUI_DrawLine(xyBoxLeft, cy, xyBoxRight, cy, BLACK, LINE_SOLID, DOT_PIXEL_1X1);   // X axis

  // Z axis vertical line
  GUI_DrawLine(zx, xyBoxTop, zx, xyBoxBottom, BLACK, LINE_SOLID, DOT_PIXEL_1X1);

  // Labels
  GUI_DisString_EN(cx - 30, xyBoxTop - 20, "X/Y Accel", &Font12, WHITE, BLACK);
  GUI_DisString_EN(zx - 25, xyBoxTop - 20, "Z Accel", &Font12, WHITE, BLACK);

  // Axis labels
  GUI_DisString_EN(xyBoxRight - 10, cy - 20, "X", &Font12, WHITE, GRAY);
  GUI_DisString_EN(cx + 5, xyBoxTop + 5, "Y", &Font12, WHITE, GRAY);
  GUI_DisString_EN(zx + 8, xyBoxTop + 5, "Z", &Font12, WHITE, GRAY);

  // Center dots
  GUI_DrawCircle(cx, cy, 3, BLACK, DRAW_FULL, DOT_PIXEL_1X1);
  GUI_DrawCircle(zx, cy, 3, BLACK, DRAW_FULL, DOT_PIXEL_1X1);
}

// ---------------- Setup / Loop ----------------
void setup() {
  // ---- Serial ----
  Serial.begin(115200);
  Serial.println("\n=== MPU6050 Accelerometer Display ===");

  // ---- I2C / MPU6050 ----
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  Serial.println("Initializing MPU6050...");
  accelgyro.initialize();
  Serial.println(accelgyro.testConnection() ? "MPU6050 connected!" : "MPU6050 connection FAILED");

  // ---- LCD ----
  Wvshr_Init();
  LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;
  LCD_Init(Lcd_ScanDir, 200);
  LCD_Clear(WHITE);
  TP_Init();

  // Calculate screen layout
  W = sLCD_DIS.LCD_Dis_Column;
  H = sLCD_DIS.LCD_Dis_Page;

  // XY display area (right side)
  xyBoxLeft = zx + 45;
  xyBoxRight = W - 10;
  xyBoxTop = 35;
  xyBoxBottom = H - 10;

  cx = (xyBoxLeft + xyBoxRight) / 2;
  cy = (xyBoxTop + xyBoxBottom) / 2;

  // Draw static UI elements once
  DrawStaticUI();

  // Initialize filtered values from first read
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  fax = ax;
  fay = ay;
  faz = az;

  Serial.println("Setup complete!\n");
}

void loop() {
  // Read acceleration and gyro data
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Apply low-pass filter to smooth jitter
  fax = ALPHA * fax + (1.0f - ALPHA) * ax;
  fay = ALPHA * fay + (1.0f - ALPHA) * ay;
  faz = ALPHA * faz + (1.0f - ALPHA) * az;

  // Map acceleration to pixel positions
  float scaleXY = 0.008f;  // Adjust for sensitivity
  float scaleZ = 0.010f;

  int px = cx + (int)(fax * scaleXY);
  int py = cy - (int)(fay * scaleXY);  // Invert Y for screen coordinates

  // Clamp to display bounds
  px = clampi(px, xyBoxLeft + 5, xyBoxRight - 5);
  py = clampi(py, xyBoxTop + 5, xyBoxBottom - 5);

  // Z axis position
  int zy = cy - (int)(faz * scaleZ);
  zy = clampi(zy, xyBoxTop + 5, xyBoxBottom - 5);

  // Check if arrows moved significantly
  bool xyMoved = abs(px - prevPx) > MOVE_THRESH || abs(py - prevPy) > MOVE_THRESH;
  bool zMoved = abs(zy - prevZy) > MOVE_THRESH;

  // Update arrows if they moved
  if (xyMoved) {
    EraseArrowXY(cx, cy, prevPx, prevPy);
    DrawArrowXY(cx, cy, px, py);

    // Redraw XY axes crossing point to fix any erasure
    GUI_DrawLine(cx - 15, cy, cx + 15, cy, BLACK, LINE_SOLID, DOT_PIXEL_1X1);
    GUI_DrawLine(cx, cy - 15, cx, cy + 15, BLACK, LINE_SOLID, DOT_PIXEL_1X1);
    GUI_DrawCircle(cx, cy, 3, BLACK, DRAW_FULL, DOT_PIXEL_1X1);

    prevPx = px;
    prevPy = py;

    // Calculate and display magnitude
    float magXY = sqrtf(fax * fax + fay * fay) / 1000.0f;  // in g's (approx)
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2fg", magXY);

    // Clear previous magnitude text area
    GUI_DrawRectangle(xyBoxRight - 50, xyBoxBottom + 5, xyBoxRight, xyBoxBottom + 20,
                      WHITE, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DisString_EN(xyBoxRight - 48, xyBoxBottom + 7, buf, &Font12, WHITE, BLUE);
  }

  if (zMoved) {
    EraseArrowZ(zx, cy, prevZy);
    DrawArrowZ(zx, cy, zy);

    // Redraw Z axis crossing point to fix any erasure
    GUI_DrawLine(zx, cy - 15, zx, cy + 15, BLACK, LINE_SOLID, DOT_PIXEL_1X1);
    GUI_DrawCircle(zx, cy, 3, BLACK, DRAW_FULL, DOT_PIXEL_1X1);

    prevZy = zy;

    // Calculate and display Z magnitude
    float magZ = faz / 1000.0f;  // in g's (approx)
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2fg", magZ);

    // Clear previous magnitude text area
    GUI_DrawRectangle(zx - 30, xyBoxBottom + 5, zx + 30, xyBoxBottom + 20,
                      WHITE, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DisString_EN(zx - 25, xyBoxBottom + 7, buf, &Font12, WHITE, BLUE);
  }

  // Update temperature periodically (not on every frame)
  unsigned long now = millis();
  if (now - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
    lastTempUpdate = now;

    int16_t tRaw = accelgyro.getTemperature();
    float tempC = (tRaw / 340.0f) + 36.53f;  // MPU6050 datasheet formula

    char buf[20];
    snprintf(buf, sizeof(buf), "%.1f C", tempC);

    // Clear previous temperature area
    GUI_DrawRectangle(W - 80, 5, W - 5, 25, WHITE, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DisString_EN(W - 75, 10, buf, &Font16, WHITE, BLACK);

    // Debug output to serial
    Serial.print("Accel: X=");
    Serial.print((int)fax);
    Serial.print(" Y=");
    Serial.print((int)fay);
    Serial.print(" Z=");
    Serial.print((int)faz);
    Serial.print(" | Temp=");
    Serial.print(tempC);
    Serial.println("C");
  }

  delay(50);  // Short delay for smooth updates
}
