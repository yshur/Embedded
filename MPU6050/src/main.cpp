#include <Arduino.h>
#include "WaveShareDemo.h"   // LCD_Driver / LCD_GUI / LCD_Touch
#include "I2Cdev.h"
#include "MPU6050.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high
//MPU6050 accelgyro(0x68, &Wire1); // <-- use for AD0 low, but 2nd Wire (TWI/I2C) object

int16_t ax, ay, az;
int16_t gx, gy, gz;

// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO


#define LED_PIN 13
bool blinkState = false;

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
  // ---- I2C / MPU6050 ----
  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif

  // initialize serial communication
  // it's really up to you depending on your project)
  Serial.begin(115200);
  Serial.println("\n=== MPU6050 Accelerometer Display ===");

  // initialize device
  Serial.println("Initializing I2C/MPU6050 devices...");
  accelgyro.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  // use the code below to change accel/gyro offset values
  /*
  Serial.println("Updating internal sensor offsets...");
  // -76	-2359	1688	0	0	0
  Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
  Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
  Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
  Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
  Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
  Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
  Serial.print("\n");
  accelgyro.setXGyroOffset(220);
  accelgyro.setYGyroOffset(76);
  accelgyro.setZGyroOffset(-85);
  Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
  Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
  Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
  Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
  Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
  Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
  Serial.print("\n");
  */

  // configure Arduino LED pin for output
  pinMode(LED_PIN, OUTPUT);

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
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

    #ifdef OUTPUT_BINARY_ACCELGYRO
        Serial.write((uint8_t)(ax >> 8)); Serial.write((uint8_t)(ax & 0xFF));
        Serial.write((uint8_t)(ay >> 8)); Serial.write((uint8_t)(ay & 0xFF));
        Serial.write((uint8_t)(az >> 8)); Serial.write((uint8_t)(az & 0xFF));
        Serial.write((uint8_t)(gx >> 8)); Serial.write((uint8_t)(gx & 0xFF));
        Serial.write((uint8_t)(gy >> 8)); Serial.write((uint8_t)(gy & 0xFF));
        Serial.write((uint8_t)(gz >> 8)); Serial.write((uint8_t)(gz & 0xFF));
    #endif

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);

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

    Serial.print("a/g:\t px=");
    Serial.print(px); Serial.print("\t py=");
    Serial.print(py); Serial.print("\t zy=");
    Serial.print(zy); Serial.print("\t prevPx=");
    Serial.print(prevPx); Serial.print("\t gy=");
    Serial.print(gy); Serial.print("\t prevZy=");
    Serial.println(prevZy);

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

      #ifdef OUTPUT_READABLE_ACCELGYRO
          // display tab-separated accel/gyro x/y/z values
          Serial.print("a/g:\t ax=");
          Serial.print(ax); Serial.print("\t ay=");
          Serial.print(ay); Serial.print("\t az=");
          Serial.print(az); Serial.print("\t gx=");
          Serial.print(gx); Serial.print("\t gy=");
          Serial.print(prevPy); Serial.print("\t prevPy=");
          Serial.println(gz);

          // Debug output to serial
          Serial.print("Accel: X(fax)=");
          Serial.print((int)fax);
          Serial.print(" Y(fay)=");
          Serial.print((int)fay);
          Serial.print(" Z(faz)=");
          Serial.print((int)faz);
          Serial.print(" | Temp=");
          Serial.print(tempC);
          Serial.println("C");
      #endif
    }
    delay(100);
}
