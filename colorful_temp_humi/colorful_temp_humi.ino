#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <DHT.h>
#include <qrcode.h>
#include <math.h>
#include <time.h>

// Myanmar Time (UTC+6:30)
const long GMT_OFFSET_SEC = 6 * 3600 + 30 * 60;
const int DAYLIGHT_OFFSET_SEC = 0;

unsigned long lastClockUpdate = 0;

// =====================================
// WIFI
// =====================================
const char* WIFI_SSID = "ITVerse";
const char* WIFI_PASS = "itverse371997";

WebServer server(80);

// =====================================
// ST7789 240x240
// =====================================
#define TFT_SCLK 4
#define TFT_MOSI 6
#define TFT_CS   7
#define TFT_DC   5
#define TFT_RST  3

Adafruit_ST7789 tft(
  &SPI, TFT_CS, TFT_DC, TFT_RST
);

// =====================================
// DHT11
// =====================================
#define DHT_PIN  10
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// =====================================
// BUTTON
// =====================================
#define BUTTON_PIN 2
#define DEBOUNCE_MS 50

bool wifiScreen = false;
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long buttonChangedAt = 0;

// =====================================
// COLORS
// =====================================
uint16_t BG, CARD, WHITE, MUTED;
uint16_t ORANGE, CYAN, GREEN, RED, TRACK;

// =====================================
// SENSOR DATA
// =====================================
float temperature = NAN;
float humidity = NAN;

float lastTemp = NAN;
float lastHumidity = NAN;

int lastTempBar = -1;
int lastHumidityBar = -1;

String lastStatus = "";

bool sensorValid = false;
bool weatherErrorShown = false;

unsigned long lastRead = 0;
unsigned long lastWifiRetry = 0;
unsigned long lastWifiCheck = 0;

const unsigned long READ_INTERVAL = 2000;

// WIFI SCREEN CACHE
String displayedIP = "";
wl_status_t displayedWifiStatus = WL_IDLE_STATUS;

// =====================================
// WEB DASHBOARD
// =====================================
const char WEB_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32-C3 Weather Monitor</title>
<style>
*{box-sizing:border-box}
body{
 margin:0;
 padding:20px;
 background:#101827;
 color:#fff;
 font-family:Arial,sans-serif;
 min-height:100vh;
 display:flex;
 justify-content:center;
 align-items:center;
}
.container{width:100%;max-width:420px}
.header,.card{
 background:#1a293f;
 border-radius:18px;
 padding:24px;
 margin-bottom:16px;
}
h2{margin:0 0 10px}
.subtitle{color:#94a3b8;font-size:13px}
.connection{
 margin-top:12px;
 color:#4ade80;
 font-size:13px;
}
.label{
 font-size:14px;
 font-weight:bold;
 letter-spacing:1px;
}
.value{
 font-size:54px;
 font-weight:bold;
 margin:16px 0 24px;
}
.orange{color:#fb923c}
.cyan{color:#38bdf8}
.unit{font-size:25px}
.progress{
 background:#354255;
 height:9px;
 border-radius:20px;
 overflow:hidden;
}
.bar{
 height:100%;
 width:0%;
 transition:width .5s ease;
}
.temp-bar{background:#fb923c}
.hum-bar{background:#38bdf8}
.footer{
 display:flex;
 justify-content:space-between;
 padding:10px 4px;
 color:#94a3b8;
 font-size:13px;
}
#status{color:#4ade80;font-weight:bold}
</style>
</head>
<body>
<div class="container">
 <div class="header">
  <h2>Environment Monitor</h2>
  <div class="subtitle">ESP32-C3 Weather Station</div>
  <div class="connection" id="connection">Connecting...</div>
 </div>

 <div class="card">
  <div class="label orange">TEMPERATURE</div>
  <div class="value orange">
   <span id="temp">--.-</span>
   <span class="unit">&deg;C</span>
  </div>
  <div class="progress">
   <div class="bar temp-bar" id="tempBar"></div>
  </div>
 </div>

 <div class="card">
  <div class="label cyan">HUMIDITY</div>
  <div class="value cyan">
   <span id="hum">--</span>
   <span class="unit">%</span>
  </div>
  <div class="progress">
   <div class="bar hum-bar" id="humBar"></div>
  </div>
 </div>

 <div class="footer">
  <span>LIVE SENSOR</span>
  <span id="status">CONNECTING</span>
 </div>
</div>

<script>
let busy=false;

async function updateData(){
 if(busy)return;
 busy=true;

 try{
  const response=await fetch('/api/data',{
   cache:'no-store'
  });

  if(!response.ok)throw Error('Network error');

  const data=await response.json();

  document.getElementById('connection')
   .textContent='Wi-Fi Connected';

  if(!data.valid){
   document.getElementById('temp').textContent='--.-';
   document.getElementById('hum').textContent='--';
   document.getElementById('tempBar').style.width='0%';
   document.getElementById('humBar').style.width='0%';
   document.getElementById('status')
    .textContent='SENSOR ERROR';
   document.getElementById('status')
    .style.color='#ef4444';
   return;
  }

  document.getElementById('temp')
   .textContent=data.temperature.toFixed(1);

  document.getElementById('hum')
   .textContent=data.humidity.toFixed(0);

  document.getElementById('tempBar').style.width=
   Math.min(100,Math.max(0,data.temperature*2))+'%';

  document.getElementById('humBar').style.width=
   Math.min(100,Math.max(0,data.humidity))+'%';

  let status='NORMAL';
  let color='#4ade80';

  if(data.temperature<20){
   status='COOL';
   color='#38bdf8';
  }else if(data.temperature>30){
   status='HOT';
   color='#ef4444';
  }

  document.getElementById('status').textContent=status;
  document.getElementById('status').style.color=color;

 }catch(e){
  document.getElementById('connection')
   .textContent='Connection lost';
  document.getElementById('status')
   .textContent='OFFLINE';
  document.getElementById('status')
   .style.color='#ef4444';
 }finally{
  busy=false;
 }
}

updateData();
setInterval(updateData,2000);
</script>
</body>
</html>
)rawliteral";

// =====================================
// INITIALIZE COLORS
// =====================================
void setupColors() {
  BG     = tft.color565(16, 24, 39);
  CARD   = tft.color565(26, 41, 63);
  WHITE  = ST77XX_WHITE;
  MUTED  = tft.color565(165, 180, 207);
  ORANGE = tft.color565(251, 146, 60);
  CYAN   = tft.color565(56, 189, 248);
  GREEN  = tft.color565(74, 222, 128);
  RED    = ST77XX_RED;
  TRACK  = tft.color565(53, 66, 85);
}

// =====================================
// WEATHER SCREEN
// =====================================
void drawWeatherScreen() {
  tft.fillScreen(BG);
  tft.setTextWrap(false);

  // Clock header
  tft.fillRoundRect(8, 8, 224, 52, 8, CARD);

  // Temperature card
  tft.fillRoundRect(8, 68, 224, 70, 10, CARD);

  tft.setTextSize(1);
  tft.setTextColor(ORANGE);
  tft.setCursor(22, 76);
  tft.print("TEMPERATURE");

  // Humidity card
  tft.fillRoundRect(8, 146, 224, 70, 10, CARD);

  tft.setTextColor(CYAN);
  tft.setCursor(22, 154);
  tft.print("HUMIDITY");

  // Progress bar tracks
  tft.fillRect(22, 126, 192, 5, TRACK);
  tft.fillRect(22, 204, 192, 5, TRACK);

  // Footer
  tft.setTextColor(MUTED);
  tft.setCursor(12, 226);
  tft.print("LIVE SENSOR");

  lastTemp = NAN;
  lastHumidity = NAN;
  lastTempBar = -1;
  lastHumidityBar = -1;
  lastStatus = "";
  weatherErrorShown = false;

  lastClockUpdate = 0;
}
// =====================================
// SMOOTH PROGRESS BAR
// =====================================
void drawBarSmooth(
  int x,
  int y,
  int percent,
  int &oldPercent,
  uint16_t color
) {
  percent = constrain(percent, 0, 100);

  int newWidth = map(percent, 0, 100, 0, 192);

  int oldWidth = oldPercent < 0
    ? 0
    : map(oldPercent, 0, 100, 0, 192);

  if(newWidth > oldWidth) {
    tft.fillRect(
      x + oldWidth,
      y,
      newWidth - oldWidth,
      6,
      color
    );
  } else if(newWidth < oldWidth) {
    tft.fillRect(
      x + newWidth,
      y,
      oldWidth - newWidth,
      6,
      TRACK
    );
  }

  oldPercent = percent;
}

// =====================================
// SMOOTH SENSOR DISPLAY UPDATE
// =====================================
void updateWeatherDisplay(float temp, float hum) {
  if(wifiScreen) return;

  // Temperature
  if(isnan(lastTemp) || temp != lastTemp) {
    char buffer[20];

    snprintf(
      buffer,
      sizeof(buffer),
      "%-7.1f C",
      temp
    );

    tft.setTextSize(3);
    tft.setTextColor(ORANGE, CARD);
    tft.setCursor(22, 86);
    tft.print(buffer);

    drawBarSmooth(
      22, 122,
      constrain((int)(temp * 2), 0, 100),
      lastTempBar,
      ORANGE
    );

    lastTemp = temp;
  }

  // Humidity
  if(isnan(lastHumidity) || hum != lastHumidity) {
    char buffer[20];

    snprintf(
      buffer,
      sizeof(buffer),
      "%-7.0f %%",
      hum
    );

    tft.setTextSize(3);
    tft.setTextColor(CYAN, CARD);
    tft.setCursor(22, 172);
    tft.print(buffer);

    drawBarSmooth(
      22, 208,
      constrain((int)hum, 0, 100),
      lastHumidityBar,
      CYAN
    );

    lastHumidity = hum;
  }

  // Temperature status
  String status;
  uint16_t color;

  if(temp < 20) {
    status = "COOL";
    color = CYAN;
  } else if(temp <= 30) {
    status = "NORMAL";
    color = GREEN;
  } else {
    status = "HOT";
    color = RED;
  }

  if(status != lastStatus) {
    char text[12];

    snprintf(
      text,
      sizeof(text),
      "%-10s",
      status.c_str()
    );

    tft.setTextSize(1);
    tft.setTextColor(color, BG);
    tft.setCursor(155, 230);
    tft.print(text);

    lastStatus = status;
  }
}

void updateClock() {
  if (wifiScreen) return;

  static String previousTime = "";
  static String previousDate = "";

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo, 10)) {
    return;
  }

  char timeBuffer[16];
  char dateBuffer[32];

  strftime(
    timeBuffer,
    sizeof(timeBuffer),
    "%I:%M:%S %p",
    &timeinfo
  );

  strftime(
    dateBuffer,
    sizeof(dateBuffer),
    "%a, %d %b %Y",
    &timeinfo
  );

  String currentTime = String(timeBuffer);
  String currentDate = String(dateBuffer);

  // Redraw only if changed
  if (currentTime != previousTime) {
    tft.setTextSize(2);
    tft.setTextColor(WHITE, CARD);

    // Clear fixed clock area on updates
    // to prevent leftover characters
    tft.fillRect(35, 15, 190, 18, CARD);

    tft.setCursor(39, 16);
    tft.print(currentTime);

    previousTime = currentTime;
  }

  if (currentDate != previousDate) {
    tft.setTextSize(1);
    tft.setTextColor(MUTED, CARD);
    tft.fillRect(45, 43, 160, 10, CARD);
    tft.setCursor(58, 43);
    tft.print(currentDate);

    previousDate = currentDate;
  }
}

// =====================================
// SENSOR ERROR
// =====================================
void showSensorError() {
  if(wifiScreen || weatherErrorShown) return;

  // Only redraw on transition to error
  tft.fillRect(20, 82, 200, 37, CARD);
  tft.fillRect(20, 168, 200, 37, CARD);

  tft.setTextSize(2);
  tft.setTextColor(RED);

  tft.setCursor(22, 88);
  tft.print("ERROR");

  tft.setCursor(22, 174);
  tft.print("ERROR");

  tft.fillRect(22, 122, 192, 6, TRACK);
  tft.fillRect(22, 208, 192, 6, TRACK);
  tft.fillRect(155, 230, 75, 8, BG);

  weatherErrorShown = true;

  lastTemp = NAN;
  lastHumidity = NAN;
  lastTempBar = -1;
  lastHumidityBar = -1;
  lastStatus = "";
}

// =====================================
// QR CODE CALLBACK
// ESPRESSIF QR CODE API
// =====================================
void qrDisplayCallback(esp_qrcode_handle_t qr) {
  int qrSize = esp_qrcode_get_size(qr);

  const int quietZone = 4;
  const int maxArea = 144;

  int scale = maxArea / (qrSize + quietZone * 2);

  // Limit QR module scale
  if(scale > 4) scale = 4;

  if(scale < 1) {
    Serial.println("QR is too large");
    return;
  }

  int totalSize =
    (qrSize + quietZone * 2) * scale;

  int x0 = (240 - totalSize) / 2;
  int y0 = 50 + (144 - totalSize) / 2;

  // White background + quiet zone
  tft.fillRect(
    x0,
    y0,
    totalSize,
    totalSize,
    ST77XX_WHITE
  );

  int qrX = x0 + quietZone * scale;
  int qrY = y0 + quietZone * scale;

  tft.startWrite();

  for(int y = 0; y < qrSize; y++) {
    for(int x = 0; x < qrSize; x++) {
      if(esp_qrcode_get_module(qr, x, y)) {
        tft.writeFillRect(
          qrX + x * scale,
          qrY + y * scale,
          scale,
          scale,
          ST77XX_BLACK
        );
      }
    }
  }

  tft.endWrite();
}

// =====================================
// GENERATE QR FOR WEB DASHBOARD
// =====================================
void drawQRCode(const String &url) {
  esp_qrcode_config_t config = {};

  config.display_func = qrDisplayCallback;
  config.max_qrcode_version = 5;
  config.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;

  esp_err_t result = esp_qrcode_generate(
    &config,
    url.c_str()
  );

  if(result != ESP_OK) {
    Serial.println("QR generation failed");

    tft.setTextSize(1);
    tft.setTextColor(RED);
    tft.setCursor(50, 110);
    tft.print("QR failed");
  }
}

// =====================================
// WIFI INFORMATION SCREEN
// =====================================
void drawWifiScreen() {
  displayedWifiStatus = WiFi.status();

  bool connected =
    displayedWifiStatus == WL_CONNECTED;

  displayedIP = connected
    ? WiFi.localIP().toString()
    : "";

  tft.fillScreen(BG);
  tft.setTextWrap(false);

  // Header
  tft.fillRoundRect(
    8, 8, 224, 38, 8, CARD
  );

  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(42, 19);
  tft.print("WIFI INFO");

  if(connected) {
    // Dashboard URL
    String url = "http://" + displayedIP + "/";

    // QR in area y=50..194
    drawQRCode(url);

    // Bottom information
    tft.setTextSize(1);

    tft.setTextColor(GREEN);
    tft.setCursor(14, 198);
    tft.print("CONNECTED - SCAN QR");

    tft.setTextColor(WHITE);
    tft.setCursor(14, 211);
    tft.print("SSID: ");
    tft.print(WIFI_SSID);

    tft.setTextColor(MUTED);
    tft.setCursor(14, 224);
    tft.print("IP: ");

    tft.setTextColor(CYAN);
    tft.print(displayedIP);

    Serial.print("QR URL: ");
    Serial.println(url);

  } else {
    tft.setTextSize(2);
    tft.setTextColor(RED);
    tft.setCursor(30, 90);
    tft.print("DISCONNECTED");

    tft.setTextSize(1);
    tft.setTextColor(MUTED);
    tft.setCursor(58, 125);
    tft.print("Reconnecting...");

    tft.setTextColor(WHITE);
    tft.setCursor(14, 211);
    tft.print("SSID: ");
    tft.print(WIFI_SSID);

    tft.setTextColor(MUTED);
    tft.setCursor(14, 224);
    tft.print("IP: Not assigned");
  }
}

// =====================================
// BUTTON - DEBOUNCED TOGGLE
// =====================================
void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);

  if(reading != lastButtonReading) {
    buttonChangedAt = millis();
  }

  if(millis() - buttonChangedAt > DEBOUNCE_MS) {
    if(reading != stableButtonState) {
      stableButtonState = reading;

      if(stableButtonState == LOW) {
        wifiScreen = !wifiScreen;

        if(wifiScreen) {
          // Switch to WiFi information
          drawWifiScreen();
        } else {
          // Return to sensor dashboard
          drawWeatherScreen();

          if(sensorValid) {
            updateWeatherDisplay(
              temperature,
              humidity
            );
          } else {
            showSensorError();
          }
        }
      }
    }
  }

  lastButtonReading = reading;
}

// =====================================
// READ DHT11
// =====================================
void readSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  bool previouslyValid = sensorValid;

  if(isnan(h) || isnan(t)) {
    sensorValid = false;

    Serial.println("DHT11 read failed");

    showSensorError();
    return;
  }

  sensorValid = true;
  temperature = t;
  humidity = h;

  Serial.printf(
    "Temperature: %.1f C | Humidity: %.0f %%\n",
    temperature,
    humidity
  );

  if(!wifiScreen) {
    // Recover only once after sensor error
    if(!previouslyValid && weatherErrorShown) {
      drawWeatherScreen();
    }

    updateWeatherDisplay(
      temperature,
      humidity
    );
  }
}

// =====================================
// WEB API
// =====================================
void handleAPI() {
  String json = "{\"valid\":";

  json += sensorValid ? "true" : "false";

  if(sensorValid) {
    json += ",\"temperature\":";
    json += String(temperature, 1);

    json += ",\"humidity\":";
    json += String(humidity, 1);
  } else {
    json += ",\"temperature\":null";
    json += ",\"humidity\":null";
  }

  json += "}";

  server.sendHeader(
    "Cache-Control",
    "no-store"
  );

  server.send(
    200,
    "application/json",
    json
  );
}

// =====================================
// CONNECT WIFI
// =====================================
void connectWiFi() {
  WiFi.mode(WIFI_STA);

  WiFi.setAutoReconnect(true);

  // Lower TX power for C3 Mini compatibility
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting WiFi");

  unsigned long start = millis();

  while(
    WiFi.status() != WL_CONNECTED &&
    millis() - start < 15000
  ) {
    delay(300);
    Serial.print(".");
  }

  Serial.println();

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Dashboard: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed");
  }
}

// =====================================
// SETUP
// =====================================
void setup() {
  Serial.begin(115200);
  delay(500);

  // Button uses internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Start DHT11
  dht.begin();

  // Hardware SPI
  SPI.begin(
    TFT_SCLK,
    -1,
    TFT_MOSI,
    TFT_CS
  );

  // ST7789
  tft.init(240, 240);
  tft.setRotation(2);

  tft.setTextWrap(false);

  setupColors();
  drawWeatherScreen();

  // Connect WiFi
  connectWiFi();

  configTime(
  GMT_OFFSET_SEC,
  DAYLIGHT_OFFSET_SEC,
  "pool.ntp.org",
  "time.google.com"
);

  // HTTP dashboard
  server.on("/", HTTP_GET, []() {
    server.send_P(
      200,
      "text/html",
      WEB_PAGE
    );
  });

  // JSON sensor endpoint
  server.on(
    "/api/data",
    HTTP_GET,
    handleAPI
  );

  server.begin();

  Serial.println("HTTP server started");

  // First sensor reading
  readSensor();
  lastRead = millis();
}

// =====================================
// MAIN LOOP
// =====================================
void loop() {
  // Keep web dashboard working
  server.handleClient();
  if (millis() - lastClockUpdate >= 1000) {
  lastClockUpdate = millis();
  updateClock();
}

  // Button handling
  handleButton();

  // Sensor updates every 2 seconds
  if(millis() - lastRead >= READ_INTERVAL) {
    lastRead = millis();
    readSensor();
  }

  // WiFi auto-reconnection
  if(
    WiFi.status() != WL_CONNECTED &&
    millis() - lastWifiRetry >= 10000
  ) {
    lastWifiRetry = millis();

    Serial.println("Reconnecting WiFi...");
    WiFi.reconnect();
  }

  // Update WiFi screen only when
  // connection status or IP changes
  if(
    wifiScreen &&
    millis() - lastWifiCheck >= 1000
  ) {
    lastWifiCheck = millis();

    String currentIP =
      WiFi.status() == WL_CONNECTED
      ? WiFi.localIP().toString()
      : "";

    if(
      WiFi.status() != displayedWifiStatus ||
      currentIP != displayedIP
    ) {
      drawWifiScreen();
    }
  }

  delay(2);
}
