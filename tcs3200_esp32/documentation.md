# TCS3200 Color Sensor + SSD1306 OLED on ESP32

A beginner-friendly Arduino project that reads colors using the TCS3200 color sensor and displays the detected color name and RGB values on a small OLED screen.

---

## Table of Contents

1. [What This Project Does](#what-this-project-does)
2. [Components Required](#components-required)
3. [Wire Connections](#wire-connections)
4. [Circuit Diagram](#circuit-diagram)
5. [Library Installation](#library-installation)
6. [How It Works](#how-it-works)
7. [Uploading the Code](#uploading-the-code)
8. [Serial Monitor Output](#serial-monitor-output)
9. [Colors It Can Detect](#colors-it-can-detect)
10. [Troubleshooting](#troubleshooting)

---

## What This Project Does

- Points the TCS3200 sensor at a colored object
- Reads the Red, Green, and Blue light values reflected from the surface
- Calculates the dominant color
- Displays the color name (e.g., RED, GREEN, BLUE, YELLOW, WHITE) in large text on the OLED
- Shows the R, G, B numeric values (0–255) below the color name
- Prints the same information to the Arduino Serial Monitor

---

## Components Required

| # | Component | Quantity |
|---|-----------|----------|
| 1 | ESP32 Development Board (e.g., ESP32 DevKit V1) | 1 |
| 2 | TCS3200 Color Sensor Module | 1 |
| 3 | SSD1306 OLED Display (128×64, I2C, 0.96 inch) | 1 |
| 4 | Breadboard | 1 |
| 5 | Jumper Wires (Male-to-Male) | ~15 |
| 6 | USB Cable (Micro-USB or USB-C depending on your ESP32) | 1 |

> **Tip for beginners:** The TCS3200 module usually comes with 4 white LEDs on the corners that illuminate the object for more accurate readings. The SSD1306 OLED communicates over I2C (only 2 data wires needed).

---

## Wire Connections

### TCS3200 Color Sensor → ESP32

| TCS3200 Pin | ESP32 Pin | Description |
|-------------|-----------|-------------|
| VCC | 3V3 | Power supply (3.3 V) |
| GND | GND | Ground |
| S0 | D5 (GPIO 5) | Frequency scaling bit 0 |
| S1 | D18 (GPIO 18) | Frequency scaling bit 1 |
| S2 | D19 (GPIO 19) | Color filter select bit 0 |
| S3 | D15 (GPIO 15) | Color filter select bit 1 |
| OUT | D4 (GPIO 4) | Frequency output (sensor data) |
| LED (OE) | D32 (GPIO 32) | Illumination LED on/off control |

> **What are S0/S1?** They set how fast the sensor outputs pulses. This code uses 20% scaling (S0=HIGH, S1=LOW) — a good balance between speed and accuracy.
>
> **What are S2/S3?** They select which color filter (Red, Green, Blue, or Clear) the sensor reads at a given moment. The code switches between them automatically.
>
> **OUT pin:** The sensor converts light intensity into a square wave. The ESP32 measures how long each pulse is — shorter pulses mean more light of that color.

---

### SSD1306 OLED Display → ESP32

| OLED Pin | ESP32 Pin | Description |
|----------|-----------|-------------|
| VCC | 3V3 | Power supply (3.3 V) |
| GND | GND | Ground |
| SDA | D21 (GPIO 21) | I2C data line |
| SCL | D22 (GPIO 22) | I2C clock line |

> **What is I2C?** It is a communication protocol that uses only 2 wires (SDA and SCL) to transfer data. GPIO 21 and GPIO 22 are the default I2C pins on most ESP32 boards.
>
> **I2C Address:** This code uses `0x3C`, the most common address for SSD1306 OLEDs. If your display doesn't turn on, try `0x3D` (see Troubleshooting).

---

### Power Rails Summary

| ESP32 Pin | Connected To |
|-----------|-------------|
| 3V3 | TCS3200 VCC, OLED VCC |
| GND | TCS3200 GND, OLED GND |

> **Important:** Always use 3.3 V (3V3), not 5 V, to power both modules with the ESP32. The ESP32 is a 3.3 V device and its GPIO pins cannot safely handle 5 V signals.

---

## Circuit Diagram

See the included breadboard diagram image:

```
TCS3200_ESP32_OLED_bb.png
```

Refer to that image alongside the tables above when wiring your circuit.

---

## Library Installation

You need two libraries. Install them through the Arduino IDE Library Manager:

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries...**
3. Search for and install each of the following:

| Library | Author | Purpose |
|---------|--------|---------|
| **Adafruit SSD1306** | Adafruit | Controls the OLED display |
| **Adafruit GFX Library** | Adafruit | Graphics layer used by SSD1306 |

> The Adafruit GFX Library is a dependency of SSD1306 — the IDE will usually prompt you to install it automatically.

---

## How It Works

### Step 1 — Setup

- The ESP32 configures the TCS3200 control pins (S0–S3) as outputs and the OUT pin as an input.
- Frequency scaling is set to 20% by pulling S0 HIGH and S1 LOW.
- The OLED is initialized over I2C and displays a short splash screen ("TCS3200 / Color Sensor") for 2 seconds.

### Step 2 — Reading a Color Channel

The function `readChannel(s2, s3)` selects one color filter by setting S2 and S3, waits 50 ms for the sensor to settle, then measures the pulse width from the OUT pin using `pulseIn()`.

| Color Filter | S2 | S3 |
|-------------|----|----|
| Red | LOW | LOW |
| Green | HIGH | HIGH |
| Blue | LOW | HIGH |

> **Lower pulse = more light of that color reflected.** A red object reflects a lot of red light, so the red channel produces the shortest pulse.

### Step 3 — Converting to RGB (0–255)

The raw pulse values are converted to a 0–255 scale. The channel with the shortest pulse (most reflected light) is mapped to 255 and the others scale relative to it.

### Step 4 — Color Detection

The `detectColor()` function looks at the three raw pulse values and decides the color name:

- **WHITE** — all three channels have similar, short pulses (lots of light reflected evenly)
- **NO OBJ** — all channels similar but not much light reflected (nothing close to sensor)
- **RED** — red channel has the shortest pulse, green not much lower than blue
- **YELLOW** — red channel shortest, but green is also significantly lower than blue (red + green = yellow)
- **GREEN** — green channel has the shortest pulse
- **BLUE** — blue channel has the shortest pulse

### Step 5 — Display

Every ~350 ms the loop:
1. Reads all three color channels
2. Detects the color name
3. Clears the OLED and prints the color name in large text (size 2)
4. Draws a horizontal divider line
5. Prints R, G, B values in small text below
6. Sends the same data to Serial Monitor

---

## Uploading the Code

1. Connect the ESP32 to your computer via USB.
2. Open `tcs3200_esp32.ino` in Arduino IDE.
3. Go to **Tools → Board** and select your ESP32 board (e.g., **ESP32 Dev Module**).
4. Go to **Tools → Port** and select the correct COM port.
5. Click the **Upload** button (right arrow icon).
6. Wait for "Done uploading" in the status bar.

> **First time with ESP32?** You need to install the ESP32 board package first. Go to **File → Preferences**, add this URL to "Additional Boards Manager URLs":
> `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
> Then go to **Tools → Board → Boards Manager**, search "esp32", and install the package by Espressif Systems.

---

## Serial Monitor Output

Open the Serial Monitor (**Tools → Serial Monitor**) at **115200 baud** to see live output:

```
RED     R:255 G:42 B:38  raw(312,2840,2910)
RED     R:255 G:40 B:36  raw(308,2860,2935)
GREEN   R:45 G:255 B:60  raw(2720,330,2600)
WHITE   R:255 G:240 B:230  raw(120,125,128)
NO OBJ  R:90 G:88 B:92  raw(1800,1820,1790)
```

Each line shows:
- Detected color name
- Normalized RGB values (0–255)
- Raw pulse widths in microseconds `(rawR, rawG, rawB)`

---

## Colors It Can Detect

| Color | Description |
|-------|-------------|
| RED | Red objects |
| GREEN | Green objects |
| BLUE | Blue objects |
| YELLOW | Yellow objects (red + green reflected) |
| WHITE | White or very light-colored objects |
| NO OBJ | No object detected, or object too far away |
| UNKNOWN | Ambiguous reading |

> **Tips for best results:**
> - Hold the object 1–3 cm directly below the sensor
> - Use a flat, matte surface — shiny objects can cause reflections that confuse the sensor
> - Dim ambient lighting improves accuracy
> - Enable the on-board LEDs by changing `#define LED_ENABLE false` to `#define LED_ENABLE true` in the code if readings are inconsistent

---

## Troubleshooting

| Problem | Likely Cause | Solution |
|---------|-------------|----------|
| OLED stays blank | Wrong I2C address | Change `OLED_ADDRESS` from `0x3C` to `0x3D` in the code |
| OLED stays blank | SDA/SCL swapped | Check that SDA → D21 and SCL → D22 |
| Serial Monitor shows garbled text | Wrong baud rate | Set Serial Monitor to **115200** |
| Always reads "NO OBJ" | Object too far away | Bring the object to within 1–3 cm of the sensor |
| Always reads "UNKNOWN" | Loose OUT wire | Re-seat the wire on GPIO 4 |
| Upload fails | Wrong board/port selected | Verify board and COM port under the **Tools** menu |
| Upload fails | ESP32 not in flash mode | Hold the **BOOT** button on the ESP32 while clicking Upload, release after upload starts |
| Readings are noisy/unstable | Bright ambient light | Shield the sensor from overhead lighting or enable the built-in LEDs |

---

*Project by minkhant — TCS3200 Color Sensor + SSD1306 OLED on ESP32*
