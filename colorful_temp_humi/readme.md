
# ESP32-C3 Super Mini, ST7789 Display နှင့် DHT11 ဖြင့် Temperature & Humidity Monitor ပြုလုပ်ခြင်း

ဒီ Tutorial မှာ **ESP32-C3 Super Mini**, **ST7789 240×240 Color TFT Display** နဲ့ **DHT11 Temperature & Humidity Sensor** ကို အသုံးပြုပြီး အပူချိန်၊ စိုထိုင်းဆ၊ နေ့စွဲ၊ အချိန်တို့ကို ပြသပေးနိုင်တဲ့ **IoT Weather Station** အသေးစားတစ်ခု တည်ဆောက်ကြည့်ပါမယ်။ Wi-Fi Web Dashboard ပါဝင်တာကြောင့် ဖုန်းနဲ့ Computer ကနေလည်း Sensor Data ကို ကြည့်ရှုနိုင်ပါတယ်။ Push Button တစ်ခုနဲ့ Wi-Fi Information Screen ကို ပြောင်းပြီး QR Code Scan ဖတ်ကာ Webpage ကို ဝင်ရောက်နိုင်ပါတယ်။

## Project Features

- အပူချိန်ကို Celsius (°C) ဖြင့် ပြသခြင်း
- စိုထိုင်းဆကို ရာခိုင်နှုန်း (%) ဖြင့် ပြသခြင်း
- ST7789 240×240 Color TFT Dashboard နှင့် Progress Bar များ
- NTP ဖြင့် Date & Time ပြသခြင်း
- Wi-Fi ချိတ်ဆက်ပြီး Phone / Computer မှ Web Dashboard ကြည့်ရှုခြင်း
- Sensor Data ကို ၂ စက္ကန့်တစ်ကြိမ် Update ပြုလုပ်ခြင်း
- GPIO2 Push Button ဖြင့် Screen ပြောင်းခြင်း
- QR Code Scan ဖတ်ပြီး Local Web Dashboard သို့ ဝင်ရောက်ခြင်း
- Screen တစ်ခုလုံး ပြန်မဆွဲဘဲ ပြောင်းလဲသည့်နေရာများသာ Update လုပ်၍ Flickering လျှော့ချခြင်း
- Wi-Fi ပြတ်တောက်လျှင် ပြန်လည်ချိတ်ဆက်ရန် ကြိုးစားခြင်း

> **မှတ်ချက်:** DHT11 သည် ပတ်ဝန်းကျင်လေထု၏ အပူချိန်နှင့် စိုထိုင်းဆကို တိုင်းတာပေးသော Sensor ဖြစ်ပြီး ဒီ Project က အိမ်တွင်း Environment Monitor အဖြစ် အထူးသင့်တော်ပါတယ်။

## လိုအပ်သော ပစ္စည်းများ

1. ESP32-C3 Super Mini Development Board — ၁ ခု
2. ST7789 240×240 SPI Color TFT Display — ၁ ခု
3. DHT11 Temperature & Humidity Sensor Module — ၁ ခု
4. Push Button — ၁ ခု
5. Jumper Wires နှင့် Breadboard (လိုအပ်ပါက)
6. USB-C Cable

လိုအပ်သော ESP32 Board, Display နဲ့ Sensor များကို [ITVerse](https://itversemm.com) မှာ ရှာဖွေဝယ်ယူနိုင်ပါတယ်။

## Circuit ချိတ်ဆက်ပုံ (Pin Connections)

| Component         | ESP32-C3 Super Mini |
| ----------------- | ------------------- |
| ST7789 SCL / SCK  | GPIO4               |
| ST7789 SDA / MOSI | GPIO6               |
| ST7789 CS         | GPIO7               |
| ST7789 DC         | GPIO5               |
| ST7789 RES / RST  | GPIO3               |
| ST7789 VCC        | 3.3V                |
| ST7789 BLK / BL   | 3.3V                |
| ST7789 GND        | GND                 |
| DHT11 DATA        | GPIO10              |
| DHT11 VCC         | 3.3V                |
| DHT11 GND         | GND                 |
| Push Button       | GPIO2 ↔ GND        |

ST7789 Display ကို **Hardware SPI** ဖြင့် ချိတ်ဆက်ထားပါတယ်။ Push Button ကို GPIO2 နဲ့ GND ကြား ချိတ်ဆက်ပြီး Code မှာ `INPUT_PULLUP` ကို အသုံးပြုပါတယ်။ DHT11 Module အများစုမှာ Pull-up Resistor ပါဝင်ပြီးသား ဖြစ်ပါတယ်။ Bare DHT11 Sensor သုံးရင် DATA နဲ့ 3.3V ကြား **10 kΩ Pull-up Resistor** တပ်ဆင်ရန် လိုအပ်နိုင်ပါတယ်။

**သတိပြုရန်:** GPIO2 က ESP32-C3 ရဲ့ Boot-Strapping Pin ဖြစ်ပါတယ်။ Power On သို့မဟုတ် Reset လုပ်နေချိန်မှာ Button ကို ဖိမထားပါနဲ့။ Display Module ဟာ 3.3V Logic နဲ့ ကိုက်ညီမှုရှိမရှိလည်း စစ်ဆေးပါ။

## Arduino IDE ပြင်ဆင်ခြင်း

1. Arduino IDE ရဲ့ **Boards Manager** မှာ **esp32 by Espressif Systems** ကို Install ပြုလုပ်ပါ။
2. **Tools → Board → ESP32 Arduino → ESP32C3 Dev Module** (သို့မဟုတ် မိမိ Board နဲ့ ကိုက်ညီတဲ့ ESP32-C3 Board) ကို ရွေးချယ်ပါ။
3. **Library Manager** မှာ အောက်ပါ Libraries များကို Install ပြုလုပ်ပါ။
   - **Adafruit GFX Library**
   - **Adafruit ST7735 and ST7789 Library**
   - **DHT sensor library** by Adafruit
   - **Adafruit Unified Sensor**
4. `WiFi`, `WebServer`, `SPI`, `time` စတဲ့ Functions များကို ESP32 Arduino Core မှာ အသုံးပြုနိုင်ပါတယ်။ QR Code အတွက်တော့ Installed Board Package နဲ့ Library API ကိုက်ညီရပါမယ်။ ဒီ Project Sketch မှာ ရရှိနိုင်သော ESP32 Core Version များအတွက် Espressif `esp_qrcode_*` API ကို အသုံးပြုထားပါတယ်။

## Project အလုပ်လုပ်ပုံ

### 1. Temperature နှင့် Humidity ဖတ်ယူခြင်း

DHT11 Sensor က ပတ်ဝန်းကျင်လေထုရဲ့ အပူချိန်နဲ့ စိုထိုင်းဆကို တိုင်းတာပါတယ်။ ESP32-C3 က Sensor Data ကို **၂ စက္ကန့်တစ်ကြိမ်** ဖတ်ယူပြီး ST7789 Display ပေါ်မှာ ပြသပေးပါတယ်။ အပူချိန်ကို °C၊ စိုထိုင်းဆကို % ဖြင့် ပြသထားပါတယ်။

Dashboard မှာ အပူချိန်အပေါ် မူတည်ပြီး အောက်ပါ Status များကိုလည်း ပြသနိုင်ပါတယ်။

- **COOL** — 20°C အောက်
- **NORMAL** — 20°C မှ 30°C အထိ
- **HOT** — 30°C အထက်

ဒီ Status များက Dashboard အတွက် ဥပမာ သတ်မှတ်ချက်သာဖြစ်ပြီး လူတိုင်းအတွက် ကျန်းမာရေး သို့မဟုတ် ဘေးကင်းရေး အဆင့်သတ်မှတ်ချက် မဟုတ်ပါဘူး။

### 2. ST7789 Color TFT Dashboard

ST7789 Display က SPI Communication ကို အသုံးပြုပြီး Temperature နဲ့ Humidity တို့ကို သီးခြား Card များ၊ Color Text နဲ့ Progress Bar များဖြင့် ပြသပေးပါတယ်။ Sensor Value ပြောင်းသွားတိုင်း Screen တစ်ခုလုံး Clear မလုပ်ဘဲ ပြောင်းလဲတဲ့ Number နဲ့ Bar နေရာတွေကိုသာ Update လုပ်ထားတာကြောင့် **Screen Flickering** ကို လျှော့ချနိုင်ပါတယ်။

### 3. NTP ဖြင့် နေ့စွဲနှင့် အချိန် ပြသခြင်း

ESP32-C3 ကို Wi-Fi ချိတ်ဆက်ပြီး Internet NTP Server ကနေ Date & Time ကို Sync လုပ်နိုင်ပါတယ်။

- **Myanmar Time:** UTC+06:30
- **Thailand Time:** UTC+07:00

Internet ကနေ Time Sync လုပ်နိုင်ရင် External RTC Module မလိုအပ်ပါဘူး။ ဒါပေမဲ့ Power လုံးဝ ပြတ်ပြီး ပြန်ဖွင့်တဲ့အခါ မှန်ကန်တဲ့အချိန်ရဖို့ Network Time Sync ပြန်လုပ်ဖို့ လိုအပ်ပါတယ်။

### 4. Wi-Fi Web Dashboard

ESP32-C3 မှာ Wi-Fi ပါဝင်လို့ Local HTTP Web Server တစ်ခု Run နိုင်ပါတယ်။ Router က ESP32 ကို IP Address တစ်ခု သတ်မှတ်ပေးပြီးနောက် ဖုန်း သို့မဟုတ် Computer Browser ကနေ Dashboard ကို ဝင်ကြည့်နိုင်ပါတယ်။

ဥပမာ IP Address က `192.168.1.105` ဆိုရင် အောက်ပါ URL ကို ဖွင့်နိုင်ပါတယ်။

`http://192.168.1.105/`

Webpage က `/api/data` ကနေ Sensor Data ကို ၂ စက္ကန့်တစ်ကြိမ် ယူပြီး Page တစ်ခုလုံး Refresh မလုပ်ဘဲ Temperature၊ Humidity နဲ့ Status ကို Update ပြုလုပ်ပေးပါတယ်။ IP Address အမှန်က Router ပေါ်မူတည်ပြီး ပြောင်းလဲနိုင်ပါတယ်။

### 5. Push Button နှင့် QR Code

GPIO2 မှာ ချိတ်ဆက်ထားတဲ့ Button ကို အသုံးပြုပြီး Display Mode နှစ်ခု ပြောင်းနိုင်ပါတယ်။

- **ပထမတစ်ချက်နှိပ်ပါ:** Weather Dashboard မှ Wi-Fi Information Screen သို့ ပြောင်းပါမယ်။ Wi-Fi Connection Status, SSID, IP Address နဲ့ QR Code ကို ပြသပေးပါတယ်။
- **QR Code Scan ဖတ်ပါ:** QR Code ထဲမှာ `http://192.168.1.105/` လိုမျိုး Local Dashboard URL အပြည့်အစုံ ပါဝင်ပါတယ်။ Phone Camera မှာ ပေါ်လာတဲ့ Website Link ကို နှိပ်ပြီး Dashboard ဖွင့်နိုင်ပါတယ်။
- **နောက်တစ်ချက်နှိပ်ပါ:** Temperature & Humidity Screen ကို ပြန်ရောက်ပါမယ်။ Sensor နဲ့ Web Server က ဆက်လက်အလုပ်လုပ်နေမှာ ဖြစ်ပါတယ်။

Phone နဲ့ ESP32 တို့ဟာ အချင်းချင်း ဆက်သွယ်နိုင်တဲ့ Local Network ပေါ်မှာ ရှိရပါမယ်။ Guest Wi-Fi Isolation ဖွင့်ထားရင် Dashboard မပွင့်နိုင်ပါဘူး။ Phone အများစုမှာ QR Code Scan ဖတ်ရုံနဲ့ Website ကို အလိုအလျောက် မဖွင့်ဘဲ Link ကို တစ်ချက်နှိပ်ပေးရပါတယ်။

## Firmware နှင့် Configuration

Arduino Firmware မှာ DHT11 Reading, ST7789 Rendering, NTP, WebServer, JSON API, QR Code Generation နဲ့ Push Button Debouncing တို့ကို ပေါင်းစပ်ထားပါတယ်။ မိမိ Wi-Fi SSID နဲ့ Password ကို Sketch ထဲမှာ သတ်မှတ်ပေးရမှာ ဖြစ်ပြီး **Password အစစ်ကို Public GitHub Repository မှာ မတင်သင့်ပါဘူး**။

Myanmar Time အတွက် NTP Setup ဥပမာ —

```cpp
#include <time.h>

const long GMT_OFFSET_SEC = 6 * 3600 + 30 * 60;
const int DAYLIGHT_OFFSET_SEC = 0;

// Wi-Fi စတင်ပြီးနောက် ခေါ်ပါ
configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC,
           "pool.ntp.org", "time.google.com");
```

Thailand Time သုံးချင်ရင် GMT Offset ကို `7 * 3600` ပြောင်းပါ။

**Arduino Source Code:** [စမ်းသပ်ပြီးသား `.ino` File သို့မဟုတ် GitHub Repository Link ကို ဤနေရာတွင် ထည့်ပါ။]

## အဖြစ်များသော ပြဿနာများ

**Display မပေါ်ပါက** — VCC, GND, SCK, MOSI, CS, DC နဲ့ RST Wiring ကို စစ်ဆေးပါ။ ST7789 SPI Module ဟုတ်မဟုတ်၊ Resolution 240×240 ဖြစ်မဖြစ်ကိုလည်း စစ်ဆေးပါ။

**Display အရောင်မှားနေပါက** — Module ပေါ်မူတည်ပြီး `tft.invertDisplay(true)` သို့မဟုတ် `tft.invertDisplay(false)` ကို စမ်းသပ်နိုင်ပါတယ်။ 180° လှည့်ချင်ရင် `tft.setRotation(2)` ကို အသုံးပြုပါ။

**DHT11 Error ပြပါက** — DATA Pin GPIO10, Power နဲ့ Pull-up Resistor ကို စစ်ဆေးပါ။ ၂ စက္ကန့်ထက် ပိုမြန်အောင် Sensor မဖတ်သင့်ပါဘူး။

**Wi-Fi မချိတ်ဆက်နိုင်ပါက** — SSID နဲ့ Password မှန်မမှန် စစ်ဆေးပါ။ ESP32-C3 က **2.4GHz Wi-Fi** ကိုသာ Support လုပ်ပြီး 5GHz-only Network ကို မချိတ်ဆက်နိုင်ပါဘူး။

**QR Code Scan ဖတ်လို့ရပေမယ့် Webpage မပွင့်ပါက** — URL မှာ `http://` ပါဝင်မပါဝင်၊ IP Address မှန်မမှန်၊ Phone နဲ့ ESP32 တို့ Local Network အတွင်း ဆက်သွယ်နိုင်မနိုင် စစ်ဆေးပါ။ QR Code အပေါ် Text မဖုံးမိအောင် White Border (Quiet Zone) ချန်ထားပါ။

**Date & Time မပေါ်ပါက** — Wi-Fi Internet Connection၊ NTP Server နဲ့ Timezone Offset မှန်ကန်မှုကို စစ်ဆေးပါ။

## နောက်ထပ် တိုးချဲ့နိုင်သည့် Features များ

ဒီ Project ကို MQTT, Home Assistant, Sensor History Graph, Cloud Data Logging, Temperature Alert System စတဲ့ IoT Features တွေနဲ့ ဆက်လက်တိုးချဲ့နိုင်ပါတယ်။ တိုင်းတာမှု ပိုတိကျစေချင်ရင် **AHT20**, **SHT31** သို့မဟုတ် **DHT22** Sensor ကို ပြောင်းလဲအသုံးပြုနိုင်ပါတယ်။

## နိဂုံးချုပ်

ESP32-C3 Super Mini Weather Station Project က Sensor Interfacing, SPI Color Display, Wi-Fi Networking, Web Server, NTP နဲ့ QR Code တို့ကို ပေါင်းစပ်လေ့လာနိုင်တဲ့ လက်တွေ့ IoT Project တစ်ခု ဖြစ်ပါတယ်။ Push Button နဲ့ QR Code ပါဝင်တာကြောင့် IP Address ကို လက်ဖြင့် ရိုက်ထည့်စရာမလိုဘဲ Dashboard ကို အလွယ်တကူ ဝင်ကြည့်နိုင်ပါတယ်။

ESP32 Development Boards, Sensors, Displays နဲ့ DIY Electronics Components များအတွက် [ITVerse](https://itversemm.com) ကို ဝင်ရောက်ကြည့်ရှုနိုင်ပါတယ်။
