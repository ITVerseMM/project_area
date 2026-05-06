/*
  TCS3200 Color Sensor + SSD1306 OLED on ESP32

  TCS3200 wiring:
    S0  -> D5    S1  -> D18
    S2  -> D19   S3  -> D15
    OUT -> D4    VCC -> 3V3   GND -> GND
    LED -> D32

  OLED (I2C SSD1306 128x64) wiring:
    SDA -> D21   SCL -> D22
    VCC -> 3V3   GND -> GND

  Libraries required (install via Library Manager):
    - Adafruit SSD1306
    - Adafruit GFX Library
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED ----------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDRESS 0x3C
#define OLED_SDA       21
#define OLED_SCL       22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- TCS3200 ----------
#define PIN_S0   5
#define PIN_S1  18
#define PIN_S2  19
#define PIN_S3  15
#define PIN_OUT  4
#define PIN_LED 32

// Set to true to turn on the illumination LED, false to turn it off
#define LED_ENABLE  false

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  pinMode(PIN_S0,  OUTPUT);
  pinMode(PIN_S1,  OUTPUT);
  pinMode(PIN_S2,  OUTPUT);
  pinMode(PIN_S3,  OUTPUT);
  pinMode(PIN_OUT, INPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LED_ENABLE ? HIGH : LOW);

  // Frequency scaling 20%
  digitalWrite(PIN_S0, HIGH);
  digitalWrite(PIN_S1, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED init failed");
    for (;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println("TCS3200");
  display.setTextSize(1);
  display.setCursor(20, 40);
  display.println("Color Sensor");
  display.display();
  delay(2000);
}

// ---------- Read one channel ----------
uint32_t readChannel(uint8_t s2, uint8_t s3) {
  digitalWrite(PIN_S2, s2);
  digitalWrite(PIN_S3, s3);
  delay(50);
  return pulseIn(PIN_OUT, LOW, 100000UL);
}

// ---------- Map pulse to 0-255 (relative to brightest channel) ----------
// minPulse = the lowest pulse reading (most light = 255)
uint8_t toRGB(uint32_t pulse, uint32_t minPulse) {
  if (pulse == 0 || minPulse == 0) return 0;
  float val = (float)minPulse / (float)pulse * 255.0f;
  return (uint8_t)constrain((int)val, 0, 255);
}

// ---------- Detect color from raw pulses ----------
// Lower pulse = more light reflected = dominant color
String detectColor(uint32_t rawR, uint32_t rawG, uint32_t rawB) {
  if (rawR == 0) rawR = 99999;
  if (rawG == 0) rawG = 99999;
  if (rawB == 0) rawB = 99999;

  uint32_t mn = min(rawR, min(rawG, rawB));
  uint32_t mx = max(rawR, max(rawG, rawB));
  float spread = (float)(mx - mn) / (float)mx;

  // White: all channels similar AND all pulses small (lots of light)
  if (spread < 0.40f && mn < 200) return "WHITE";

  // No object: channels similar but not enough reflection
  if (spread < 0.50f) return "NO OBJ";

  // R channel dominant (lowest pulse)
  if (rawR < rawG && rawR < rawB) {
    // Yellow: G also significantly lower than B (R+G both reflected)
    if ((float)rawG / rawB < 0.70f) return "YELLOW";
    return "RED";
  }

  if (rawG < rawR && rawG < rawB) return "GREEN";
  if (rawB < rawR && rawB < rawG) return "BLUE";
  return "UNKNOWN";
}

// ---------- Main loop ----------
void loop() {
  uint32_t rawR = readChannel(LOW,  LOW);
  uint32_t rawG = readChannel(HIGH, HIGH);
  uint32_t rawB = readChannel(LOW,  HIGH);

  // Use smallest pulse as reference so brightest channel = 255
  uint32_t mn = min(rawR, min(rawG, rawB));
  if (mn == 0) mn = 1;

  uint8_t r = toRGB(rawR, mn);
  uint8_t g = toRGB(rawG, mn);
  uint8_t b = toRGB(rawB, mn);

  String colorName = detectColor(rawR, rawG, rawB);

  Serial.printf("%s  R:%d G:%d B:%d  raw(%lu,%lu,%lu)\n",
    colorName.c_str(), r, g, b, rawR, rawG, rawB);

  // ----- OLED -----
  display.clearDisplay();

  // Color name — large, centered
  display.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(colorName, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 2);
  display.println(colorName);

  display.drawFastHLine(0, 26, SCREEN_WIDTH, SSD1306_WHITE);

  // RGB values
  display.setTextSize(1);
  display.setCursor(0, 32);
  display.printf("R: %3d", r);
  display.setCursor(0, 43);
  display.printf("G: %3d", g);
  display.setCursor(0, 54);
  display.printf("B: %3d", b);

  display.display();
  delay(300);
}
