# :partly_sunny: CYD Weather Station :pager:
[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![test](https://img.shields.io/github/actions/workflow/status/nicholaswilde/cyd-weather-station/test.yaml?label=test&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/cyd-weather-station/actions/workflows/test.yaml)
[![ci](https://img.shields.io/github/actions/workflow/status/nicholaswilde/cyd-weather-station/ci.yaml?label=ci&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/cyd-weather-station/actions/workflows/ci.yaml)

A beautiful, configurable real-time weather station and desk clock built for the **ESP32 Cheap Yellow Device (CYD)** (board model ESP32-2432S028R) utilizing the **LVGL v8** graphics library, **Open-Meteo / OpenWeatherMap APIs**, and the **Catppuccin Color Theme**.

> [!WARNING]
> This project is currently in a `v0.X.X` development stage. Features and configurations are subject to change, and breaking changes may be introduced at any time.

## :star: Features

- **Dual API Integration**: 
  - **Open-Meteo API**: Out-of-the-box fallback — no API key required.
  - **OpenWeatherMap API**: Automatically used if an API key is configured.
- **Location Resolution & City Name Footer**:
  - Displays `Last Update: <time> | <city name>` centered at the bottom of the screen.
  - **Zip Code Geocoding**: Enter a US Zip Code (e.g. `90210`); coordinates are resolved on boot.
  - **Reverse Geocoding**: When using coordinates + Open-Meteo, the city is resolved using Nominatim OSM. OWM resolves and returns the city name natively.
- **3-Day Forecast View**: Swipe to a dedicated Forecast tab showing daily high/low temperature and weather condition icons.
- **Swipe Navigation**: Swipe left/right anywhere on the screen to switch between the Current, Forecast, and Settings tabs.
- **Dynamic Weather Icons**: A large (48px) custom weather glyph maps weather codes to condition icons, dynamically colored using the active Catppuccin palette.
- **Interactive Settings Tab**: Touch-configurable settings persisted to flash across reboots:
  - **Temperature Unit**: Toggle between Celsius (°C) and Fahrenheit (°F).
  - **Catppuccin Theme Flavor**: Choose between Mocha, Macchiato, Frappé, or Latte — the full UI redraws instantly in the selected palette.
  - **Auto Brightness**: Toggle automatic backlight dimming/brightening driven by the onboard LDR light sensor (GPIO 34).
  - **Manual Brightness**: Slider to set a fixed screen brightness level (when Auto is off).
  - **Timezone Offset**: `–` / `+` buttons to set a GMT offset (–12 to +14) for the NTP clock.
  - **SD Log**: Enable/disable weather logging to a microSD card.
  - **Screenshot Server**: Enable/disable the remote screenshot HTTP server.
- **Auto-Brightness Control**: Uses the LDR photoresistor (GPIO 34) with an EMA filter feeding LEDC PWM (GPIO 21) to smoothly adapt screen brightness to ambient light.
- **NTP Time Synchronization**: Connects to NTP on boot and keeps a live clock in the header bar, respecting the configured timezone offset.
- **RGB LED Status Indicator**: Onboard RGB LED (GPIO 4/16/17) provides Wi-Fi status feedback (blinking blue for connecting, solid green for connected, fast red for disconnected, slow purple blink for AP Mode) and a brief weather-condition color pulse on updates.
- **Wi-Fi AP Captive Portal Fallback**:
  - Automatically hosts an open Soft AP (`cyd-weather-station-<mac_short>`) if connection fails or times out after 30 seconds on boot.
  - Runs a captive portal configuration web server and DNS redirector on `192.168.4.1` for selecting networks and setting Wi-Fi credentials.
  - Dynamically displays step-by-step connection instructions (SSID name and IP address) directly on the screen while AP Mode is active.
  - Colors the header Wi-Fi icon **Mauve** when in setup configuration mode.
- **SD Card Weather Logging**:
  - Automatically mounts a microSD card on boot and appends weather records (timestamp, temperature, humidity, wind speed, wind direction, weather code) to `/weather_history.csv` on the root of the card.
  - Automatic formatting fallback to FAT32 on mount failure.
- **Screenshot Capture**:
  - **Remote via HTTP**: `GET /screenshot` streams a pixel-perfect 24-bit BMP of the current screen directly over Wi-Fi.
  - **Physical button**: Press the BOOT button (GPIO 0) to save a timestamped BMP to the SD card as `/screenshot_YYYYMMDD_HHMMSS.bmp`.
  - Zero large-allocation design — screen tiles are intercepted from the LVGL flush callback and written directly to file.
  - Toggle the screenshot server on/off from the **Settings tab** (`Scr Srv`).

## :hammer_and_wrench: Hardware Requirements

- **ESP32 Cheap Yellow Device (CYD)**: ESP32-2432S028R — 2.8″ 320×240 ILI9341 LCD with XPT2046 resistive touch.
- **Onboard Sensors**: LDR photoresistor (GPIO 34), Backlight PWM (GPIO 21), RGB LED (GPIO 4/16/17), BOOT button (GPIO 0).
- **Storage**: MicroSD card slot (compatible with standard FAT32 formatted cards).
- Micro-USB cable for power and programming.

## :floppy_disk: MicroSD Card Logging & Auto-Formatting

The weather station periodically logs weather reports to a microSD card in CSV format.

> [!WARNING]
> **Auto-Formatting Warning:** If the inserted microSD card fails to mount (e.g., if it is formatted as exFAT or uses a GPT partition scheme), the firmware will **automatically format the card to FAT32** on boot. This will **permanently delete all existing data** on the card.
> 
> To prevent data loss, ensure that any card you insert is either empty, or pre-formatted as **FAT32** with a **Master Boot Record (MBR)** partition scheme (GUID/GPT partition tables are not supported).

## :camera: Screenshots

The device supports capturing the current screen as a standard 24-bit BMP image via two methods:

### Remote HTTP capture

> [!NOTE]
> The screenshot server must be enabled in the **Settings tab** (`Scr Srv` toggle) and Wi-Fi must be connected. The device IP is printed to serial on boot: `[WiFi] Connected! IP address: <IP>`.

```bash
# Save to file
curl http://<DEVICE_IP>/screenshot -o screenshot.bmp

# View inline (if ImageMagick is installed)
curl -s http://<DEVICE_IP>/screenshot | display -
```

### Physical BOOT button capture

Press the **BOOT button** (GPIO 0) on the CYD board. The screenshot is saved to the SD card root as `/screenshot_YYYYMMDD_HHMMSS.bmp` (NTP-synced timestamp).

Serial output confirms the save:
```
[System] BOOT button pressed. Taking screenshot...
[Screenshot] Capture started: /screenshot_20260711_135852.bmp
[Screenshot] Capture complete.
```

---

## :rocket: Getting Started

### 1. Secrets Setup

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
   #define AP_PASSWORD ""

   // (Optional) Set your API key here to use OpenWeatherMap instead of Open-Meteo.
   // If left empty (""), Open-Meteo API will be used as a fallback.
   #define OPENWEATHERMAP_API_KEY "YOUR_OPENWEATHERMAP_API_KEY"
   ```

### 2. Configuration

Static settings (location, update interval) live in [`config/config.h`](config/config.h). Runtime user preferences (units, brightness, theme, timezone) are changed via the on-device **Settings tab** and saved to flash.

**Location:**
```cpp
#define USE_ZIP_CODE      true
#define WEATHER_ZIP_CODE  "90210"

// Fallback coordinates (used when USE_ZIP_CODE is false)
#define WEATHER_API_LATITUDE  "37.7749"
#define WEATHER_API_LONGITUDE "-122.4194"
```

**Weather update interval:**
```cpp
#define WEATHER_UPDATE_INTERVAL_MINS 15
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

### 3. Build & Upload

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
| **Timezone** | GMT offset (– / + buttons, range –12 to +14). |
| **DST** | Toggle Daylight Saving Time on/off (adds 1 hour to NTP offset when enabled). |
| **SD Log** | Toggle SD card weather logging. Disabled automatically if no card is inserted. |
| **Scr Srv** | Toggle the remote screenshot HTTP server on/off. |

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
| **Lint Check** | `task check` | `pio check` | Runs `cppcheck` static analysis. |
| **Setup Node** | `task setup:node` | `npm install` | Installs `lv_font_conv` for font conversion. |
| **Generate Font** | `task font:generate` | `npm run font:generate` | Rebuilds the weather icons C source from TTF. |
| **Clean** | `task clean` | `pio run --target clean` | Removes build output and temp files. |

## :balance_scale: License

[Apache License 2.0](LICENSE)

## :writing_hand: Author

This project was started in 2026 by [Nicholas Wilde](https://github.com/nicholaswilde/).
