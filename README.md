# :partly_sunny: CYD Weather Station :pager:
[![Coveralls](https://img.shields.io/badge/dynamic/xml?url=https%3A%2F%2Fcoveralls.io%2Frepos%2Fgithub%2Fnicholaswilde%2Fcyd-weather-station%2Fbadge.svg%3Fbranch%3Dmain&query=%2F%2F*%5Blocal-name()%3D'text'%5D%5Blast()%5D&label=Coveralls&style=for-the-badge&logo=coveralls)](https://coveralls.io/github/nicholaswilde/cyd-weather-station?branch=main)
[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![test](https://img.shields.io/github/actions/workflow/status/nicholaswilde/cyd-weather-station/test.yaml?label=test&style=for-the-badge&branch=main&logo=github-actions)](https://github.com/nicholaswilde/cyd-weather-station/actions/workflows/test.yaml)
[![ci](https://img.shields.io/github/actions/workflow/status/nicholaswilde/cyd-weather-station/ci.yaml?label=ci&style=for-the-badge&logo=github-actions)](https://github.com/nicholaswilde/cyd-weather-station/actions/workflows/ci.yaml)

A beautiful, configurable real-time weather station and desk clock built for the **ESP32 Cheap Yellow Device (CYD)** family (such as the 2.8" Resistive **ESP32-2432S028R**, 2.8" Capacitive **ESP32-2432W328C**, and 3.5" Capacitive **ESP32-3248S035C**) utilizing the **LVGL v8** graphics library, **Open-Meteo / OpenWeatherMap APIs**, and the **Catppuccin Color Theme**.

> [!WARNING]
> This project is currently in a `v0.X.X` development stage. Features and configurations are subject to change, and breaking changes may be introduced at any time.

## :star: Features

- **Dual API Integration**: 
  - **Open-Meteo API**: Out-of-the-box fallback — no API key required.
  - **OpenWeatherMap API**: Automatically used if an API key is configured.
- **Location Resolution & City Name Footer**:
  - Displays `Last Update: <time> | <city name>` centered at the bottom of the screen.
  - **Zip Code Geocoding**: Enter a US Zip Code (e.g. `90210`); coordinates are resolved on boot.
  - **IP Geolocation Fallback**: Automatically falls back to resolving location via IP geolocation (using `ip-api.com`) on boot if Zip Code and coordinates are left blank.
  - **Reverse Geocoding**: When using coordinates + Open-Meteo, the city is resolved using Nominatim OSM. OWM resolves and returns the city name natively.
- **3-Day Forecast View**: Swipe to a dedicated Forecast tab showing daily high/low temperature and weather condition icons.
- **Swipe Navigation**: Swipe left/right anywhere on the screen to switch between the Current, Forecast, and Settings tabs.
- **Dynamic Weather Icons**: A large (48px) custom weather glyph maps weather codes to condition icons, dynamically colored using the active Catppuccin palette.
- **Interactive Settings Tab**: Touch-configurable settings persisted to flash across reboots:
  - **Temperature Unit**: Toggle between Celsius (°C) and Fahrenheit (°F).
  - **Catppuccin Theme Flavor**: Choose between Mocha, Macchiato, Frappé, or Latte — the full UI redraws instantly in the selected palette.
  - **Auto Brightness**: Toggle automatic backlight dimming/brightening driven by the onboard LDR light sensor (GPIO 34) with smooth non-linear perceptual scaling.
  - **Manual Brightness**: Slider to set a fixed screen brightness level with natural $\gamma = 2.2$ gamma correction.
  - **Timezone**: `–` / `+` buttons to cycle through common POSIX timezone presets for automatic DST handling.
  - **SD Log**: Enable/disable weather logging to a microSD card.
  - **SD Cache**: Enable/disable weather caching to a microSD card (restores UI offline).
  - **Screenshot Server**: Enable/disable the remote screenshot HTTP server.
  - **MQTT**: Toggle publishing weather variables to MQTT topics with a configurable base topic (default `cyd/`).
  - **MQTT Integration & Remote Control**:
    - **Configurable Base Topic**: Customize the base topic (e.g. `home/living_room/weather/`) via the web settings portal.
    - **Home Assistant Auto-Discovery**: Automatically exposes entities for weather sensors (temp, humidity, wind speed/direction, condition, city), device diagnostics (uptime, heap, RSSI, IP, version, MAC), configuration controls (screen & LED brightness, auto brightness, theme flavor, units, orientation, update interval, screensaver timeout, status LED, SD log/cache), and device restart button.
    - **Bidirectional Remote Control**: Change any setting or command via MQTT topics with immediate on-screen and Home Assistant state synchronization.
    - **LWT & Robust Reconnection**: Reports online/offline availability via Last Will and Testament (LWT) on `<base_topic>status` with MAC-based unique client IDs, paced discovery message queueing, and exponential reconnection backoff (5s to 2min).
  - **Screensaver**: Enable/disable screensaver mode (smoothly fades backlight using gamma interpolation and displays clock after inactivity).
  - **Sleep Schedule**: Enable/disable automatic screen sleep during a configured time frame (e.g. 22:00 to 07:00).
  - **Screen Orientation**: Choose between Landscape, Portrait, Landscape Rev, or Portrait Rev—the entire UI dynamically scales/stacks, header height dynamically increases to 60px in portrait to fit a wrapped two-line title without overlaps, and touch coordinates update instantly.
- **Auto-Brightness & Gamma Scaling**: Uses the LDR photoresistor (GPIO 34) with an EMA filter and a perceptual $\gamma = 2.2$ gamma curve feeding LEDC PWM (GPIO 21) to smoothly adapt screen brightness to ambient light.
- **NTP Time Synchronization**: Connects to NTP on boot and keeps a live clock in the header bar, respecting the configured POSIX timezone.
- **RGB LED Status Indicator**: Onboard RGB LED (GPIO 4/16/17) with gamma-corrected brightness provides Wi-Fi status feedback (blinking blue for connecting, solid green for connected, fast red for disconnected, slow purple blink for AP Mode) and a brief weather-condition color pulse on updates.
- **Web Dashboard & Settings Portal**:
  - Access `http://<DEVICE_IP>/` in any browser for a central Catppuccin-themed dashboard.
  - Includes direct navigation links to Device Settings (`/settings`), Firmware Updates (`/update`), Live Screenshots (`/screenshot`), Clear SD Logs (`/clear_logs`), Clear SD Cache (`/clear_cache`), and Factory Reset (`/reset`).
  - Settings portal automatically redirects back to the main dashboard after saving settings once the device comes back online.
- **Wi-Fi AP Captive Portal Fallback**:
  - Automatically hosts an open Soft AP (`cyd-weather-station-<mac_short>`) if connection fails or times out after 30 seconds on boot.
  - Runs a captive portal configuration web server and DNS redirector on `192.168.4.1` for selecting networks, setting Wi-Fi credentials, and configuring your location (Zip Code or Coordinates).
  - Runs background async Wi-Fi scans after AP initialization to prevent client disconnections (common on ESP32 during active/blocking scans).
  - Web interface `/scan` triggers background scanning with a friendly redirect loader screen, displaying updated networks on completion.
  - Dynamically displays step-by-step connection instructions (SSID name and IP address) directly on the screen while AP Mode is active.
  - Colors the header Wi-Fi icon **Mauve** when in setup configuration mode.
- **SD Card Weather Logging**:
  - Automatically mounts a microSD card on boot and appends weather records (timestamp, temperature, humidity, wind speed, wind direction, weather code) to `/weather_history.csv` on the root of the card.
  - Automatic formatting fallback to FAT32 on mount failure.
- **Screenshot Capture**:
  - **Remote via HTTP**: `GET /screenshot` streams a pixel-perfect 24-bit BMP of the current screen directly over Wi-Fi.
  - **Physical button**: Press and hold the BOOT button (GPIO 0) for 2 seconds to save a timestamped BMP to the SD card as `/screenshot_YYYYMMDD_HHMMSS.bmp`. A single quick press of the BOOT button triggers an immediate weather refresh.
  - Zero large-allocation design — screen tiles are intercepted from the LVGL flush callback and written directly to file.
  - Toggle the screenshot server on/off from the **Settings tab** (`API Srv`).
- **SD Offline Cache Recovery**:
  - Automatically caches the latest retrieved weather data to `/weather_cache.json` on the microSD card.
  - If the device boots without Wi-Fi, it restores the weather screen from the cache instead of displaying blank widgets, displaying a `⚠️ Offline` status badge in the header.
- **Wireless OTA Firmware Updates**:
  - Easily update firmware wirelessly by navigating to `/update` in a browser when connected to the configuration AP (`http://192.168.4.1/update`) or the local network IP (`http://<DEVICE_IP>/update` when the Screenshot Server is enabled).
  - Portal features a responsive Catppuccin-themed HTML interface, drag-and-drop binary selector, dynamic progress bar, error handling, and auto-reboot upon completion.


## :hammer_and_wrench: Hardware Requirements

- **ESP32 Cheap Yellow Device (CYD)**:
  - **CYD 2.8" (Resistive)**: ESP32-2432S028R — 2.8″ 320×240 ILI9341 LCD with XPT2046 resistive touch.
  - **CYD 2.8" (Capacitive)**: ESP32-JC2432W328C — 2.8″ 320×240 ST7789 LCD with CST816 capacitive touch.
  - **CYD 3.5" (Capacitive)**: ESP32-3248S035C — 3.5″ 480×320 ST7796 LCD with GT911/CST820 capacitive touch.
- **Onboard Sensors**: LDR photoresistor (GPIO 34), Backlight PWM (GPIO 21), RGB LED (GPIO 4/16/17), BOOT button (GPIO 0).
- **Storage**: MicroSD card slot (compatible with standard FAT32 formatted cards).
- Micro-USB cable for power and programming.

## :electric_plug: Hardware Setup / Sensor Wiring

To read local temperature and humidity, you can wire a sensor directly to the CYD board. 

> [!NOTE]
> Currently, the **DHT22 (AM2302)**, **DHT11**, and **SHT40 (I2C)** temperature and humidity sensors are supported.

### DHT11 / DHT22 Wiring
Connect the DHT sensor to the **CN1** breakout port:
- **VCC**: 3V3
- **GND**: GND
- **Data/Signal**: IO22

    ```text
    +------------------------------+
    |                              |
    |                          +------+
    |                          |  P3  |
    |                          +------+
    |                              |
    |                       +---------+
    |                       |   CN1   |      +--------------+
    |                       |  GND[X] |------| [X]GND       |
    |                       | IO22[X] |------| [X]SIG / OUT |
    |                       | IO27[ ] |      |              |
    |                       | 3.3V[X] |------| [X]VCC       |
    |                       +---------+      +--------------+ 
    |                              |           DHT11 / DHT22
    |                          +------+     
    |                          |  SD  |
    |                          | CARD |
    |                          +------+
    |                              |
    | +--+ +-------+ +-----------+ |
    +-|  |-| USB C |-| MICRO USB |-+
      +--+ +-------+ +-----------+ 
             ESP32-2432S028R  
    ```

    ```text
    +------------------------------+
    |                              |
    |                          +------+
    |                          |  P3  |
    |                          +------+
    |                              |
    |                       +---------+
    |                       |   CN1   |      +--------------+
    |                       |  GND[X] |------| [X]GND       |
    |                       | IO22[X] |------| [X]Sig / OUT |
    |                       | IO21[ ] |      |              |
    |                       | 3.3V[X] |------| [X]VCC       |
    |                       +---------+      +--------------+ 
    |                              |           DHT11 / DHT22
    |                          +------+         
    |                          |  SD  |
    |                          | CARD |
    |                          +------+
    |                              |
    | +--+           +-----------+ |
    +-|  |-----------| MICRO USB |-+
      +--+           +-----------+ 
             ESP32-3248S035C  
    ```

### SHT40 (I2C) Wiring
Connect the SHT40 sensor to the **CN1** breakout port (do not use connector P3 as it lacks a 3.3V power supply):
- **VCC**: 3V3
- **GND**: GND
- **SDA**: IO27 (on `cyd_28r`) or IO21 (on `cyd_35c` / `cyd_28c`)
> [!NOTE]
> On the `cyd_28r` board, the CN1 connector breaks out **IO27**. IO27 must be used for SDA to prevent conflicts with the display backlight on IO21.
> On the `cyd_35c` and `cyd_28c` boards, the CN1 connector breaks out **IO21**. IO21 must be used for SDA to prevent conflicts with the display backlight on IO27.
- **SCL**: IO22

    ```text
    +------------------------------+
    |                              |
    |                          +------+
    |                          |  P3  |
    |                          +------+
    |                              |
    |                       +---------+
    |                       |   CN1   |      +--------+
    |                       |  GND[X] |------| [X]GND |
    |                       | IO22[X] |------| [X]SCL |
    |                       | IO27[X] |------| [X]SDA |
    |                       | 3.3V[X] |------| [X]VCC |
    |                       +---------+      | [ ]3Vo | 
    |                              |         +--------+
    |                          +------+         SHT40
    |                          |  SD  |
    |                          | CARD |
    |                          +------+
    |                              |
    | +--+ +-------+ +-----------+ |
    +-|  |-| USB C |-| MICRO USB |-+
      +--+ +-------+ +-----------+ 
             ESP32-2432S028R  
    ```

    ```text
    +------------------------------+
    |                              |
    |                          +------+
    |                          |  P3  |
    |                          +------+
    |                              |
    |                       +---------+
    |                       |   CN1   |      +--------+
    |                       |  GND[X] |------| [X]GND |
    |                       | IO22[X] |------| [X]SCL |
    |                       | IO21[X] |------| [X]SDA |
    |                       | 3.3V[X] |------| [X]VCC |
    |                       +---------+      | [ ]3Vo | 
    |                              |         +--------+
    |                          +------+         SHT40
    |                          |  SD  |
    |                          | CARD |
    |                          +------+
    |                              |
    | +--+           +-----------+ |
    +-|  |-----------| MICRO USB |-+
      +--+           +-----------+ 
             ESP32-3248S035C  
    ```

### Local Sensor Calibration

Sensors mounted near the CYD board often pick up residual heat radiated by the ESP32 chip and LCD backlight (self-heating), resulting in temperature readings that are slightly high and relative humidity readings that are slightly low. You can calibrate both values using single-point linear offsets via the on-device Settings screen, the Web Settings UI, or MQTT/Home Assistant.

#### 1. Temperature Calibration (Known Accurate Secondary Device)
1. Place a trusted, known-accurate digital thermometer right beside the CYD's local sensor.
2. Allow both devices to stabilize in the room for at least 30–60 minutes away from direct drafts, sunlight, or airflow.
3. Compare the readings:
   $$\text{Temperature Offset} = T_{\text{reference}} - T_{\text{CYD}}$$
   *Example:* If your reference thermometer reads $72.0^\circ\text{F}$ and the CYD reads $74.5^\circ\text{F}$, set the **Temperature Offset** to `-2.5`.
4. Enter this offset in the **Settings** screen (or Web UI / MQTT).

#### 2. Humidity Calibration (Saturated Salt Test Method)
For accurate relative humidity calibration, the standard and recommended method is the **Saturated Salt (NaCl) Slurry Test** (see [wikiHow: How to Test a Hygrometer](https://www.wikihow.com/Test-a-Hygrometer) for a detailed visual guide), which creates an exact **75% Relative Humidity** sealed environment at room temperature:
1. In a small, shallow container (like a bottle cap or small cup), mix ordinary table salt with a few drops of water until it forms a thick, wet slush or slurry (all salt should be wet, but with no free-standing liquid water on top).
2. Place the salt container along with the sensor (or entire board if wired directly) inside an airtight container or zip-top freezer bag. *Ensure no salt water directly touches the sensor.*
3. Seal the bag completely and let it sit at room temperature for at least 6–8 hours to reach equilibrium.
4. Check the humidity reading without unsealing the bag:
   $$\text{Humidity Offset} = 75\% - H_{\text{CYD}}$$
   *Example:* If the CYD reads $79\%$ inside the sealed 75% RH salt container, set the **Humidity Offset** to `-4.0%`.
5. Enter this offset in the **Settings** screen (or Web UI / MQTT).

## :floppy_disk: MicroSD Card Logging & Auto-Formatting

The weather station periodically logs weather reports to a microSD card in CSV format.

> [!WARNING]
> **Auto-Formatting Warning:** If the inserted microSD card fails to mount (e.g., if it is formatted as exFAT or uses a GPT partition scheme), the firmware will **automatically format the card to FAT32** on boot. This will **permanently delete all existing data** on the card.
> 
> To prevent data loss, ensure that any card you insert is either empty, or pre-formatted as **FAT32** with a **Master Boot Record (MBR)** partition scheme (GUID/GPT partition tables are not supported).

## :camera: Screenshots

### Device UI Showcase

| Screen | Landscape (320x240) | Portrait (240x320) |
|---|---|---|
| **Current Weather** | ![Landscape Current](screenshots/landscape_current.png) | ![Portrait Current](screenshots/portrait_current.png) |
| **3-Day Forecast** | ![Landscape Forecast](screenshots/landscape_forecast.png) | ![Portrait Forecast](screenshots/portrait_forecast.png) |
| **24-Hour Forecast** | ![Landscape Hourly](screenshots/landscape_hourly.png) | ![Portrait Hourly](screenshots/portrait_hourly.png) |
| **System Settings** | ![Landscape Settings](screenshots/landscape_settings.png) | ![Portrait Settings](screenshots/portrait_settings.png) |

The device supports capturing the current screen as a standard 24-bit BMP image via two methods:

### Remote Settings, Web Dashboard & API

> [!NOTE]
> The Settings Web UI, Dashboard, and Configuration API are available while the device is connected to Wi-Fi. The device IP is printed to serial on boot: `[WiFi] Connected! IP address: <IP>`.

**Web Dashboard:**
Navigate to `http://<DEVICE_IP>/` in any browser to access the central Catppuccin-themed dashboard landing page. From here, you can access:
- **⚙️ Device Settings (`/settings`)**: Configure all device parameters at runtime (Units, Theme, Screen & LED Brightness, Timezone, Weather Update Interval, Screensaver Timeout, Static IP, AP Password, API Server, SD Settings, MQTT, etc.) and save them without reflashing.
- **🔄 Firmware Update (`/update`)**: Flash new firmware binaries wirelessly.
- **📸 View Screenshot (`/screenshot`)**: Stream a pixel-perfect image of the current screen (if enabled). Includes inline status indicators and help prompts.
- **🗑️ Clear SD Logs (`/clear_logs`)**: Delete the weather history CSV file (`/weather_history.csv`) stored on the microSD card.
- **🗑️ Clear SD Cache (`/clear_cache`)**: Delete the weather cache JSON file (`/weather_cache.json`) stored on the microSD card.
- **⚠️ Factory Reset (`/reset`)**: Completely erase NVS settings and saved Wi-Fi credentials to reboot the device into AP Setup mode.

**Capture Screenshot:**
```bash
# Save to file
curl http://<DEVICE_IP>/screenshot -o screenshot.bmp

# View inline (if ImageMagick is installed)
curl -s http://<DEVICE_IP>/screenshot | display -
```

**Get Configuration Settings:**
Retrieve the current settings in JSON format:
```bash
curl http://<DEVICE_IP>/api/config
```

Example JSON response:
```json
{
  "unit_system": 2,
  "brightness": 75,
  "auto_brightness": false,
  "timezone": "UTC0",
  "theme_flavor": 1,
  "sd_logging_enabled": true,
  "screenshot_server_enabled": false,
  "api_server_enabled": true,
  "screen_orientation": 1,
  "led_enabled": true,
  "led_brightness": 60,
  "mqtt_enabled": true,
  "mqtt_server": "192.168.1.88",
  "mqtt_port": 1883,
  "mqtt_user": "user",
  "mqtt_password": "password",
  "mqtt_base_topic": "cyd/",
  "wifi_ssid": "Your_SSID",
  "wifi_password": "Your_Password",
  "sd_cache_enabled": true,
  "screensaver_enabled": false,
  "screensaver_timeout": 300000,
  "sleep_schedule_enabled": false,
  "sleep_start_time": "22:00",
  "sleep_end_time": "07:00",
  "weather_update_interval": 15,
  "static_ip_enabled": false,
  "static_ip": "192.168.1.8",
  "static_gateway": "192.168.1.1",
  "static_subnet": "255.255.255.0",
  "static_dns": "1.1.1.1",
  "ap_password": "",
  "zip_code": "90210",
  "city_code": "",
  "latitude": "",
  "longitude": "",
  "owm_api_key": "",
  "ntp_server": "pool.ntp.org",
  "use_24_hour_format": false,
  "local_sensor_enabled": true,
  "local_sensor_type": 1,
  "local_sensor_update_interval": 60,
  "local_sensor_temp_offset": 0.0,
  "local_sensor_hum_offset": 0.0
}
```

**Update Configuration Settings:**
Update any subset of device settings dynamically by posting a JSON payload:
```bash
curl -X POST -H "Content-Type: application/json" \
  -d '{"brightness": 80, "theme_flavor": 2}' \
  http://<DEVICE_IP>/api/config
```

Response:
```json
{"status":"ok"}
```

### Physical BOOT button capture

Press and hold the **BOOT button** (GPIO 0) on the CYD board for 2 seconds. The screenshot is saved to the SD card root as `/screenshot_YYYYMMDD_HHMMSS.bmp` (NTP-synced timestamp). A single quick press of the BOOT button triggers an immediate weather refresh.

Serial output confirms the save:
```
[System] BOOT button pressed. Taking screenshot...
[Screenshot] Capture started: /screenshot_20260711_135852.bmp
[Screenshot] Capture complete.
```

---

## :rocket: Getting Started

### 1. Quick Install (Pre-compiled Binaries)

**Option A: Web Flasher (Easiest)**
You can easily flash the pre-compiled firmware directly from your browser using the [ESP32 Web Flasher](https://nicholaswilde.io/esp32-web-flasher/). This requires a Web Serial compatible browser (like Chrome or Edge).

**Option B: Terminal Flash Script**
Alternatively, you can flash the device directly from your terminal using the provided flash script. Replace `/dev/ttyUSB0` with your actual serial port. By default, it flashes the `cyd_28r` version, but you can specify the device as the first argument.

```bash
# Flash the default cyd_28r (2.8" resistive 320x240 screen)
bash -c "$(curl -fsSL https://raw.githubusercontent.com/nicholaswilde/cyd-weather-station/main/scripts/flash.sh)" _ cyd_28r /dev/ttyUSB0

# Or flash the cyd_28c version (2.8" capacitive 320x240 screen)
bash -c "$(curl -fsSL https://raw.githubusercontent.com/nicholaswilde/cyd-weather-station/main/scripts/flash.sh)" _ cyd_28c /dev/ttyUSB0

# Or flash the cyd_35c version (3.5" capacitive 480x320 screen)
bash -c "$(curl -fsSL https://raw.githubusercontent.com/nicholaswilde/cyd-weather-station/main/scripts/flash.sh)" _ cyd_35c /dev/ttyUSB0
```

> [!WARNING]
> Running a script directly from the internet with `bash -c "$(curl...)"` is a potential security risk. Always review the script's source code before executing it to ensure it is safe. You can view the script [here](https://github.com/nicholaswilde/cyd-weather-station/blob/main/scripts/flash.sh).

### 2. Build from Source

#### Secrets Setup

Wi-Fi credentials and API keys live in a Git-ignored secrets file to prevent committing them.

1. Copy the template:
   ```bash
   task init
   ```
   *(Or manually: `cp config/secrets.h.example config/secrets.h`)*

2. Edit `config/secrets.h`:
   ```cpp
   #define WIFI_SSID     "Your_WiFi_Network"
   #define WIFI_PASSWORD "Your_WiFi_Password"

   // (Optional) Secure the configuration AP with a password (at least 8 chars).
   // Leave blank ("") or comment out to run an open Access Point.
   // This can also be changed at runtime via the Web Settings UI.
   #define AP_PASSWORD ""

   // (Optional) Set your API key here to use OpenWeatherMap instead of Open-Meteo.
   // If left empty (""), Open-Meteo API will be used as a fallback.
   #define OPENWEATHERMAP_API_KEY "YOUR_OPENWEATHERMAP_API_KEY"
   ```

#### Configuration

Static settings (location, update interval) live in [`config/config.h`](config/config.h). Runtime user preferences (units, brightness, theme, timezone) are changed via the on-device **Settings tab** and saved to flash.

**Location:**
Location (Zip Code, City Code, or Coordinates) is configured dynamically via the **Captive Portal Wi-Fi Manager** when the device boots in AP mode. You can pre-configure the fallback defaults in `config/config.h`:
```cpp
// Default values (if not configured via Wi-Fi Setup)
#define WEATHER_ZIP_CODE  "90210"
#define WEATHER_API_LATITUDE  ""
#define WEATHER_API_LONGITUDE ""

// City Code (For OpenWeatherMap only)
#define WEATHER_CITY_CODE ""
```
*(Note: If you leave these fields empty in both the code and the Wi-Fi Setup page, the device will automatically detect your location via IP Geolocation).*

**Weather update interval:**
You can change the update interval dynamically via the Web Settings UI, or set the default fallback here:
```cpp
#define WEATHER_UPDATE_INTERVAL_MINS 15
```

**Screensaver:**
The screensaver timeout can be adjusted dynamically via the Web Settings UI. Defaults are configured here:
```cpp
#define SCREENSAVER_ENABLED     true
#define SCREENSAVER_TIMEOUT_MS  300000 // 5 minutes (in milliseconds)
```

**Sleep Schedule:**
The sleep schedule can be configured dynamically via the Web Settings UI or MQTT. Defaults are configured in `config/config.h`:
```cpp
#define DEFAULT_SLEEP_SCHEDULE_ENABLED false
#define DEFAULT_SLEEP_START_TIME       "22:00"
#define DEFAULT_SLEEP_END_TIME         "07:00"
```

**Static IP:**
Static IP can be configured directly in the Web Settings UI at runtime. Alternatively, you can uncomment the static IP settings block in `config/config.h` to set the default fallback values. If kept blank or commented out, the device will default to DHCP:
```cpp
// #define STATIC_IP          "192.168.1.100"
// #define STATIC_GATEWAY     "192.168.1.1"
// #define STATIC_SUBNET      "255.255.255.0"
// #define STATIC_DNS         "1.1.1.1"
```

**Display performance** — tune animation smoothness vs. touch responsiveness:
```cpp
// Height of the LVGL draw buffer (pixel rows).
// Larger = smoother animation; too large = unresponsive touch.
// Sweet spot on CYD hardware: 25–35.
#define DISPLAY_DRAW_BUF_ROWS 30

// How often LVGL redraws changed areas (ms). Range: 10–30.
#define DISPLAY_REFR_PERIOD_MS 20

// How often LVGL polls the touchscreen (ms). Keep <= DISPLAY_REFR_PERIOD_MS.
#define DISPLAY_INDEV_READ_PERIOD_MS 10

// Duration of the tab-switch swipe animation (ms). LVGL default: 300.
#define DISPLAY_SWIPE_ANIM_MS 150
```

#### Build & Upload

```bash
task build    # Compile firmware
task upload   # Flash to the connected CYD board
task monitor  # Open serial monitor (115200 baud)
```

---

## :gear: Settings Tab Reference

All settings below are configured by touch on the device and saved to flash:

| Setting | Description |
| :--- | :--- |
| **Unit (C/F)** | Toggle between Celsius and Fahrenheit. |
| **Auto Light** | Enable/disable LDR-driven automatic backlight control. |
| **Brightness** | Manual backlight level slider (disabled when Auto Light is on). |
| **Theme** | Catppuccin flavor selector — Mocha / Macchiato / Frappé / Latte. Full UI redraws on change. |
| **Timezone** | POSIX timezone selector (– / + buttons) to automatically handle DST offsets. |
| **SD Log** | Toggle SD card weather logging. Disabled automatically if no card is inserted. |
| **SD Cache** | Toggle SD card weather caching. |
| **API Srv** | Toggle the remote screenshot & configuration HTTP API server on/off. |
| **Local Sensor** | Toggle reading local temperature/humidity sensor on/off. |
| **Sensor Interval** | Slider adjusting local sensor read interval (seconds) with live preview. |
| **Temp Offset** | Numerical input with themed on-screen numeric keypad to set temperature offset (supports decimals). |
| **Hum Offset** | Numerical input with themed on-screen numeric keypad to set humidity offset (supports decimals). |
| **Status LED** | Toggle RGB status LED feedback on/off. |
| **LED Brightness** | Manual RGB status LED brightness level slider. |
| **Scr Saver** | Toggle the screensaver on/off. |
| **Sleep Sched** | Toggle the sleep schedule on/off. |
| **MQTT** | Toggle publishing weather variables to MQTT topics. |

---

### POSIX Timezone Configuration

The CYD Weather Station uses POSIX timezone strings to natively handle Daylight Saving Time (DST) changes. You can configure this via the device UI, the Wi-Fi Captive Portal, or pre-configure the default in `config.h`.

> [!NOTE]
> **Why POSIX strings instead of timezone names?**
> The ESP32's C library natively parses POSIX strings to calculate local time and DST transitions. It does *not* include the massive IANA timezone database (tzdata) required to map names like `America/New_York` to offsets. Sticking to POSIX strings avoids embedding a bloated lookup table into flash memory, keeping the firmware extremely efficient.

For a comprehensive list of POSIX timezone strings for all regions, see the [IBM POSIX Timezone Reference](https://www.ibm.com/docs/en/aix/7.2?topic=concepts-posix-time-zone-format) or community-maintained lists like [this Gist](https://gist.github.com/alwynallan/24d96091655391107939).

The device UI Settings tab allows cycling through the following common presets:

| Region | Preset Display | POSIX String |
| :--- | :--- | :--- |
| **UTC** | UTC | `UTC0` |
| **London** | London | `GMT0BST,M3.5.0/1,M10.5.0` |
| **Central Europe** | CET | `CET-1CEST,M3.5.0,M10.5.0/3` |
| **Eastern Europe** | EET | `EET-2EEST,M3.5.0/3,M10.5.0/4` |
| **US Eastern** | US East | `EST5EDT,M3.2.0,M11.1.0` |
| **US Central** | US Central | `CST6CDT,M3.2.0,M11.1.0` |
| **US Mountain** | US Mount. | `MST7MDT,M3.2.0,M11.1.0` |
| **US Pacific** | US Pacific | `PST8PDT,M3.2.0,M11.1.0` |
| **US Alaska** | US Alaska | `AKST9AKDT,M3.2.0,M11.1.0` |
| **US Hawaii** | US Hawaii | `HST10` |
| **AU Eastern** | AU East | `AEST-10AEDT,M10.1.0,M4.1.0/3` |
| **AU Central** | AU Central | `ACST-9:30ACDT,M10.1.0,M4.1.0/3` |
| **AU Western** | AU West | `AWST-8` |

---

### :satellite: MQTT Topics & Home Assistant Integration

When MQTT is enabled in settings, the CYD Weather Station connects to your configured MQTT broker and provides state telemetry, Home Assistant auto-discovery, system diagnostics, and bidirectional remote control commands.

> [!NOTE]
> All telemetry, state, and command topics are prefixed with your configurable **MQTT Base Topic** (default: `cyd/`). Home Assistant auto-discovery topics always use the standard `homeassistant/` root prefix.

#### Telemetry & State Topics

| Topic | Description | Example / Values |
| :--- | :--- | :--- |
| `<base_topic>status` | Connection availability (LWT) | `online` / `offline` |
| `<base_topic>weather/temperature` | Current temperature | `72` |
| `<base_topic>weather/humidity` | Current relative humidity | `45` |
| `<base_topic>weather/wind_speed` | Current wind speed | `8` |
| `<base_topic>weather/wind_direction` | Current wind direction | `NW` |
| `<base_topic>weather/status` | Current weather condition | `Partly Cloudy` |
| `<base_topic>weather/city` | Resolved city name | `Seattle` |
| `<base_topic>system/uptime` | Device uptime (seconds) | `3600` |
| `<base_topic>system/free_heap` | Free heap memory (bytes) | `184320` |
| `<base_topic>system/wifi_rssi` | Wi-Fi signal strength (dBm) | `-58` |
| `<base_topic>system/ip` | Device IP address | `192.168.1.150` |
| `<base_topic>system/version` | Firmware version string | `v0.1.22` |
| `<base_topic>system/mac` | Device MAC address | `B0:CB:D8:DA:77:5C` |
| `<base_topic>settings/brightness` | Screen brightness percentage | `0`–`100` |
| `<base_topic>settings/led_brightness` | Status LED brightness percentage | `0`–`100` |
| `<base_topic>settings/auto_brightness` | Auto brightness switch state | `ON` / `OFF` |
| `<base_topic>settings/screensaver` | Screensaver switch state | `ON` / `OFF` |
| `<base_topic>settings/theme` | Catppuccin theme flavor | `Mocha` / `Macchiato` / `Frappe` / `Latte` |
| `<base_topic>settings/units` | Unit system | `Imperial` / `Metric` |
| `<base_topic>settings/screen_orientation`| Display orientation | `Landscape` / `Portrait` / `Landscape Rev` / `Portrait Rev` |
| `<base_topic>settings/update_interval` | Weather update interval (mins) | `15` |
| `<base_topic>settings/screensaver_timeout`| Screensaver timeout (mins) | `5` |
| `<base_topic>settings/sleep_schedule` | Sleep schedule switch state | `ON` / `OFF` |
| `<base_topic>settings/sleep_start` | Sleep schedule start time | `22:00` |
| `<base_topic>settings/sleep_end` | Sleep schedule end time | `07:00` |
| `<base_topic>settings/led` | Status LED enabled switch state | `ON` / `OFF` |
| `<base_topic>settings/sd_log` | SD logging enabled switch state | `ON` / `OFF` |
| `<base_topic>settings/sd_cache` | SD caching enabled switch state | `ON` / `OFF` |

#### Remote Control Topics

| Topic | Payload | Action |
| :--- | :--- | :--- |
| `<base_topic>command/brightness` | `0`–`100` | Adjusts the screen backlight brightness percentage and saves setting. |
| `<base_topic>command/led_brightness` | `0`–`100` | Adjusts the status LED brightness percentage (0-100% mapped to hardware 0-255). |
| `<base_topic>command/auto_brightness`| `ON` / `OFF` / `1` / `0` | Enables or disables ambient light-based automatic brightness. |
| `<base_topic>command/screensaver` | `ON` / `OFF` / `1` / `0` | Enables or disables the screensaver. |
| `<base_topic>command/sleep_schedule` | `ON` / `OFF` / `1` / `0` | Enables or disables the sleep schedule. |
| `<base_topic>command/sleep_start` | `HH:MM` | Sets the sleep schedule start time (24-hour format). |
| `<base_topic>command/sleep_end` | `HH:MM` | Sets the sleep schedule end time (24-hour format). |
| `<base_topic>command/theme` | `Mocha` / `Macchiato` / `Frappe` / `Latte` | Changes the active Catppuccin theme flavor. |
| `<base_topic>command/units` | `Imperial` / `Metric` | Changes the temperature and wind speed unit system. |
| `<base_topic>command/screen_orientation`| `Landscape` / `Portrait` / `Landscape Rev` / `Portrait Rev` | Changes display orientation dynamically. |
| `<base_topic>command/update_interval` | `1`–`120` | Sets the weather fetch update interval in minutes. |
| `<base_topic>command/screensaver_timeout`| `1`–`60` | Sets screensaver activation inactivity timeout in minutes. |
| `<base_topic>command/led` | `ON` / `OFF` / `1` / `0` | Enables or disables the onboard status RGB LED. |
| `<base_topic>command/sd_log` | `ON` / `OFF` / `1` / `0` | Enables or disables SD card weather history logging. |
| `<base_topic>command/sd_cache` | `ON` / `OFF` / `1` / `0` | Enables or disables SD card weather caching. |
| `<base_topic>command/reboot` | `REBOOT` / `1` / `true` / `ON` | Reboots the ESP32 weather station. |

#### Home Assistant MQTT Discovery
On connection, the device automatically registers itself as `CYD Weather Station <short_mac>` with full device hierarchy and exposes the following entities to Home Assistant:
- **Weather Sensors**: Temperature (°F/°C), Humidity (%), Wind Speed (mph/m/s), Wind Direction, Weather Condition, City Name.
- **Diagnostics Sensors**: Connection Status (binary sensor), Uptime (s), Free Memory (B), Wi-Fi Signal (dBm), IP Address, Firmware Version, MAC Address.
- **Controls & Sliders (Numbers)**: Screen Brightness (0–100%), LED Brightness (0–100%), Weather Update Interval (1–120 min), Screensaver Timeout (1–60 min).
- **Configuration Switches**: Auto Brightness, Screensaver, Sleep Schedule, Status LED, SD Log, SD Cache.
- **Text Controls**: Sleep Start Time, Sleep End Time.
- **Dropdown Selectors**: Theme Flavor, Unit System, Screen Orientation.
- **Buttons**: Device Reboot.

#### Connection Management & Reconnect Backoff
If the MQTT broker goes offline or becomes unreachable, the weather station will automatically attempt to reconnect using an **exponential backoff** strategy (doubling the reconnect delay from 5s up to a maximum of 2 minutes) to prevent network congestion. Reconnect intervals reset back to 5s upon a successful connection. Auto-discovery payloads are throttled with asynchronous FreeRTOS pacing to ensure reliable network transmission without packet loss.

---

## :computer: Development

This project is built with **PlatformIO** and supports both ESP32 hardware builds and native desktop mock testing via a CMock/Unity test suite.

### Custom Weather Icon Font

Weather icons use Erik Flowers' Weather Icons font converted to LVGL C source via `lv_font_conv`.

1. Install Node.js dependencies:
   ```bash
   task setup:node
   ```
2. Regenerate the font:
   ```bash
   task font:generate
   ```

### Command Reference

| Action | Task Command | PlatformIO Equivalent | Description |
| :--- | :--- | :--- | :--- |
| **Initialize** | `task init` | `cp config/secrets.h.example config/secrets.h` | Copies the secrets template. |
| **Build** | `task build` | `pio run` | Compiles the ESP32 firmware. |
| **Upload** | `task upload` | `pio run --target upload` | Flashes firmware to the CYD board. |
| **Monitor** | `task monitor` | `pio device monitor` | Opens the serial monitor at 115200 baud. |
| **Native Tests** | `task test` | `pio test -e native` | Runs mock unit tests on the host. |
| **Pre-Flight Check** | `task test:preflight` | — | Builds all environments and runs unit tests. |
| **API Tests** | `task test:api` | — | Runs live JSON API integration tests against the device. |
| **Web Health Check** | `task test:web` | — | Verifies all HTTP endpoints respond and do not crash. |
| **MQTT Tests** | `task test:mqtt` | — | Verifies MQTT broker connectivity and command handling. |
| **OTA Update** | `task update:ota` | — | Compiles and flashes firmware wirelessly over the network. |
| **Lint Check** | `task check` | `pio check` | Runs `cppcheck` static analysis. |
| **Setup Node** | `task setup:node` | `npm install` | Installs `lv_font_conv` for font conversion. |
| **Generate Font** | `task font:generate` | `npm run font:generate` | Rebuilds the weather icons C source from TTF. |
| **Clean** | `task clean` | `pio run --target clean` | Removes build output and temp files. |

## :wrench: Troubleshooting

If you encounter any issues with your screen or the software, please review the solutions below or [create an issue](https://github.com/nicholaswilde/cyd-weather-station/issues) on GitHub if you are still stuck.

### Inverted Colors
If the colors on your display appear inverted (for example, dark themes like `Mocha`, `Macchiato`, or `Frappe` display with bright white backgrounds while the light `Latte` theme appears dark), this is due to hardware display controller differences between CYD batches. You can easily resolve it by:
- Flashing the alternative inversion release for your board (e.g., `cyd_28r` vs `cyd_28r_inv`, `cyd_28c` vs `cyd_28c_inv`, or `cyd_35c` vs `cyd_35c_inv`).
- Or, if building from source, switching to the corresponding `_inv` environment in PlatformIO or toggling the `-D TFT_INVERSION_ON=1` build flag.

### Garbled or Scrambled Screen
If the UI renders distorted, noisy, or with stretched fonts across the display, ensure you flashed the firmware matching your exact display size and controller:
- **2.8" Resistive (ILI9341)**: `cyd_28r` / `cyd_28r_inv` (240x320)
- **2.8" Capacitive (ESP32-2432W328C)**: `cyd_28c` / `cyd_28c_inv` (240x320)
- **3.5" Capacitive (ST7796)**: `cyd_35c` / `cyd_35c_inv` (320x480)

Flashing a 3.5" image onto a 2.8" display (or vice versa) will result in garbled screens due to framebuffer and resolution mismatches.

### Flashing, Erasing Flash & Boot Reset
- **Erasing Flash**: Running a full `erase_flash` wipes all non-volatile storage (NVS), which erases previously saved Wi-Fi credentials and configuration. The device will reboot into Access Point (`192.168.4.1`) setup mode. Reflashing without erasing preserves your Wi-Fi credentials and saved runtime settings.
- **Bootloader Mode**: If flashing manually via serial (`esptool.py`), the ESP32 might remain in bootloader mode after writing. Press the physical **RESET / EN** button (or power cycle the unit) to boot into the application.

### Local Temperature & Humidity Issues / SHT40 Troubleshooting
Standard CYD boards (including `ESP32-2432W328C` and `CYD-2432S028`) do **not** come with an onboard ambient temperature/humidity sensor. If no external sensor is wired to the expansion header, the local sensor readings will remain empty. You can disable the local sensor tile in Settings (`/settings`) or connect a supported sensor (DHT11, DHT22, or SHT40).

If you are using an **SHT40** sensor and it is failing to initialize or read:
- **Use the CN1 Port**: Ensure you plug the sensor into the **CN1** connector (near the USB port). Connector **P3** (near the SD card slot) lacks a 3.3V power rail and cannot power the sensor.
- **SDA Pin Selection**: On `cyd_28r`, ensure **SDA** is wired to **IO27**. GPIO 21 is used for display backlight PWM (`TFT_BL`). On `cyd_35c` and `cyd_28c`, ensure **SDA** is wired to **IO21**. IO27 is used for the display backlight. Connecting I2C to the backlight pin will interfere with I2C communications and cause the screen backlight to dim or turn off.
- **Swap SDA & SCL Lines**: If the SHT40 sensor is not recognized or reports `[Sensor] Failed to read from SHT40 sensor!`, try switching/swapping the **SDA** and **SCL** wiring pins (ensure `SDA = 27` (or 21) and `SCL = 22`).

### RGB / BGR Swap
If your screen has red and blue colors swapped, it means the display expects a BGR color order instead of RGB. A dedicated release is not currently provided for this variation, but you can fix it by building from source: simply append `-D TFT_RGB_ORDER=TFT_BGR` to your environment's `build_flags` in `platformio.ini`.

### Touch Calibration (Resistive Screens)
The 2.8" resistive version (`cyd_28r`) uses a resistive touch layer which can sometimes be misaligned or mapped to inverted coordinates depending on the batch. If touches are registering in the wrong place, you may need to run a touch calibration sketch to determine the correct offsets for your specific screen. Capacitive variants (`cyd_28c`, `cyd_35c`) do not require calibration.

### Backlight Pin and Polarity Differences
If your screen is completely black but the board seems to be running, it might be a backlight pin mapping issue. While most boards use GPIO 21 or 27 for the backlight (`TFT_BL`), some obscure batches might map it differently. Furthermore, some boards require `TFT_BACKLIGHT_ON=LOW` instead of `HIGH`. 

### Different Driver Chips
Most 2.8" screens use the `ILI9341` driver. However, occasionally manufacturers substitute it with an `ST7789V` driver without changing the board's appearance. This will result in a completely blank or noisy screen until you change the driver flag in your `platformio.ini` from `-D ILI9341_DRIVER=1` to `-D ST7789_DRIVER=1`.

### SD Card Formatting
The CYD board can be picky about SD cards. If you encounter a "Failed to initialize SD card" error:
- Ensure the microSD card is formatted as **FAT32** (not exFAT).
- Try to use an SD card that is 32GB or smaller.
- Ensure it is inserted fully before booting the device.

### Power Delivery Glitches
Intermittent touch issues, screen flickering, or spontaneous reboots are frequently caused by inadequate power. Ensure you are using a high-quality USB power supply and a good USB cable that can supply the burst current the ESP32 and backlight need simultaneously.

### Audio Output / DAC Pins
If you ever intend to use the speaker connector on the back, be aware that the DAC pin mapping changes between board revisions. Sometimes it's mapped to GPIO 25, and sometimes GPIO 26.

## :balance_scale: License

[Apache License 2.0](LICENSE)

## :writing_hand: Author

This project was started in 2026 by [Nicholas Wilde](https://github.com/nicholaswilde/).
