# ESP32-C3 Super Mini Weather Station with ST7789, DHT11, Wi-Fi Dashboard and QR Code

Build a colorful temperature and humidity monitoring station with an **ESP32-C3 Super Mini**, **ST7789 240×240 SPI TFT display**, and **DHT11 sensor**. This project also provides a live date/time clock, a phone-friendly Wi-Fi dashboard, and a button-operated QR code screen for quick access.

## Features

- Temperature in °C and relative humidity in %
- Colorful TFT display with progress bars and temperature status
- Current date and time via internet NTP synchronization
- Local Wi-Fi dashboard, accessible on a phone or computer
- Automatic sensor readings every 2 seconds without reloading the webpage
- GPIO2 button to switch between weather and Wi-Fi information screens
- QR code with the local web dashboard URL
- Partial display updates to reduce flickering
- Automatic Wi-Fi reconnect attempts

> **Note:** The DHT11 measures room conditions; this is an indoor environmental monitor rather than an outdoor meteorological weather station.

## Components

1. ESP32-C3 Super Mini development board × 1
2. ST7789 240×240 SPI TFT display × 1
3. DHT11 temperature and humidity sensor module × 1
4. Momentary push button × 1
5. Jumper wires and optional breadboard
6. USB-C power/programming cable

Find development boards, displays, and sensors at [ITVerse](https://itversemm.com).

## Pin Connections

| Component | ESP32-C3 Super Mini |
|---|---|
| ST7789 SCL / SCK | GPIO4 |
| ST7789 SDA / MOSI | GPIO6 |
| ST7789 CS | GPIO7 |
| ST7789 DC | GPIO5 |
| ST7789 RES / RST | GPIO3 |
| ST7789 VCC | 3.3V |
| ST7789 BLK / BL | 3.3V |
| ST7789 GND | GND |
| DHT11 DATA | GPIO10 |
| DHT11 VCC | 3.3V |
| DHT11 GND | GND |
| Push button | GPIO2 ↔ GND |

The sketch uses the ESP32-C3's hardware SPI interface. The push button connects to ground and uses the MCU's internal pull-up (`INPUT_PULLUP`). On a bare four-pin DHT11, add a pull-up resistor (typically 10 kΩ) between DATA and 3.3V; many three-pin modules include it already.

**Important:** GPIO2 is a boot-strapping pin. Do not hold the button down during reset or power-on. Use a display module compatible with **3.3V logic**; verify your particular display's power requirements.

## Arduino IDE Setup

1. Install the **esp32 by Espressif Systems** board package from Arduino IDE's Boards Manager.
2. Select **ESP32C3 Dev Module** (or your exact ESP32-C3 Super Mini definition) and its USB port.
3. Install these libraries using Library Manager:
   - **Adafruit GFX Library**
   - **Adafruit ST7735 and ST7789 Library**
   - **DHT sensor library** by Adafruit
   - **Adafruit Unified Sensor**
4. The project also uses the ESP32 Arduino core's `WiFi`, `WebServer`, `SPI`, and `time` facilities. For QR generation, match the QR library/API to the ESP32 core version: the project sketch uses Espressif's `esp_qrcode_*` API where available.

## How It Works

### Read temperature and humidity

The DHT11 produces temperature and relative humidity readings. The ESP32-C3 requests an update every two seconds and presents the latest readings on the ST7789. The sample firmware categorizes temperatures as **COOL** (below 20°C), **NORMAL** (20–30°C), or **HOT** (above 30°C). These are illustrative dashboard labels, not medical or safety limits.

### Render a smoother TFT dashboard

The 240×240 ST7789 display communicates over SPI. The dashboard has separate cards for temperature and humidity. Instead of clearing the whole screen every time, the program updates changing numbers and only changed progress-bar segments, reducing noticeable flashing.

### Synchronize date and time

With Wi-Fi access, the ESP32 can retrieve time from an NTP server. Choose the offset for your location: **Myanmar UTC+06:30** or **Thailand UTC+07:00**. No separate RTC module is needed while the device can synchronize; after a full power loss, accurate time requires synchronization again.

### Serve the web dashboard

The ESP32-C3 runs an HTTP server on port 80. Once connected to Wi-Fi, it receives a local IP address from the router. For example, if the address is `192.168.1.105`, visit:

`http://192.168.1.105/`

The web page fetches JSON readings from `/api/data` roughly every two seconds and updates its temperature, humidity, progress bars, and status without reloading the whole page.

### Switch screens with GPIO2

- **First button press:** Switch from the sensor dashboard to a Wi-Fi information page displaying SSID, connection status, IP address, and a QR code.
- **Scan the QR code:** It encodes a complete local HTTP URL such as `http://192.168.1.105/`. Tap the website prompt on your phone to open it.
- **Next button press:** Return to the weather screen. Sensor measurement and the web server continue running.

Your phone and ESP32 must be on networks that can communicate with each other; usually that means the same Wi-Fi LAN. Guest-network isolation can prevent access. Scanning a QR code does not always automatically launch a website—most phones require a tap.

## Firmware and Configuration

The project firmware combines DHT11 sensor reading, ST7789 graphics, NTP time, a `WebServer` endpoint, QR generation, and debounced button input. Configure the Wi-Fi SSID and password in the sketch, and avoid publishing a real Wi-Fi password in a public repository.

Example time setup for **Myanmar**:

```cpp
#include <time.h>

const long GMT_OFFSET_SEC = 6 * 3600 + 30 * 60;
const int DAYLIGHT_OFFSET_SEC = 0;

// Call after starting Wi-Fi:
configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC,
           "pool.ntp.org", "time.google.com");
```

For **Thailand**, use `7 * 3600` as the GMT offset.

**Full Arduino source:** [Add a link to your tested `.ino` sketch or GitHub repository here.]

## Troubleshooting

**The display is blank.** Check 3.3V power and the SCK, MOSI, CS, DC, and RST lines. Confirm that the module is an SPI ST7789 with the expected 240×240 resolution.

**The display is upside down or colors look wrong.** Try `tft.setRotation(2)` for a 180° rotation. Depending on your panel, `tft.invertDisplay(true)` or `tft.invertDisplay(false)` may give the correct color appearance.

**DHT11 reads fail.** Verify DATA is wired to GPIO10, check the sensor power and pull-up, and keep measurements at least two seconds apart.

**Wi-Fi does not connect.** Check the configured SSID/password and router settings. The ESP32-C3 supports 2.4 GHz Wi-Fi, not a 5 GHz-only network.

**The QR code scans but the page doesn't load.** Check that the code includes `http://`, that the IP address is current, and that the phone can access devices on the same LAN. Leave a white quiet zone around the QR code—don't draw text over it.

**The date/time is missing.** Verify that Wi-Fi provides internet access to the NTP server and that the correct timezone offset is configured.

## Improvements to Try Next

You can expand this into an IoT monitoring system by adding MQTT or Home Assistant integration, a temperature history chart, cloud logging, alerts, or a more accurate sensor such as the **AHT20**, **SHT31**, or **DHT22**.

## Conclusion

This ESP32-C3 weather monitor combines embedded programming, a color SPI display, environmental sensing, time synchronization, and local web development in one beginner-friendly build. Its GPIO2 button and QR code make accessing the dashboard convenient without typing an IP address.

Explore more electronics and DIY project components at [ITVerse](https://itversemm.com).
