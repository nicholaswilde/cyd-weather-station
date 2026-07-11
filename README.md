# CYD Weather Station

A beautiful, configurable real-time weather station and desk clock built for the **ESP32 Cheap Yellow Device (CYD)** (board model ESP32-2432S028R) utilizing the **LVGL v8** graphics library, **Open-Meteo Weather API**, and the **Catppuccin Color Theme**.

## Features

- **Real-Time Weather Details**: Fetches current temperature, humidity, and condition status from the free [Open-Meteo API](https://open-meteo.com/) — no API key required.
- **Location Resolution**:
  - **Zip Code Geocoding**: Enter a US Zip Code (e.g. `90210`); coordinates are resolved on boot via the Open-Meteo Geocoding API.
  - **Coordinates**: Enter latitude and longitude directly as a fallback.
- **3-Day Forecast View**: Swipe to a dedicated Forecast tab showing daily high/low temperature and weather condition icons.
- **Swipe Navigation**: Swipe left/right anywhere on the screen to switch between the Current, Forecast, and Settings tabs.
- **Dynamic Weather Icons**: A large (48px) custom weather glyph maps WMO weather codes to condition icons (clear, cloudy, rainy, snowy, stormy), dynamically colored using the active Catppuccin palette (sunny yellow, rainy blue, cloudy lavender, stormy mauve, etc.).
- **Interactive Settings Tab**: Touch-configurable settings persisted to flash across reboots:
  - **Temperature Unit**: Toggle between Celsius (°C) and Fahrenheit (°F).
  - **Catppuccin Theme Flavor**: Choose between Mocha, Macchiato, Frappé, or Latte — the full UI redraws instantly in the selected palette.
  - **Auto Brightness**: Toggle automatic backlight dimming/brightening driven by the onboard LDR light sensor (GPIO 34).
  - **Manual Brightness**: Slider to set a fixed screen brightness level (when Auto is off).
  - **Timezone Offset**: `–` / `+` buttons to set a GMT offset (–12 to +14) for the NTP clock.
- **Auto-Brightness Control**: Uses the LDR photoresistor (GPIO 34) with an EMA filter feeding LEDC PWM (GPIO 21) to smoothly adapt screen brightness to ambient light.
- **NTP Time Synchronization**: Connects to NTP on boot and keeps a live clock in the header bar, respecting the configured timezone offset.
- **RGB LED Status Indicator**: The onboard RGB LED (GPIO 4/16/17) provides Wi-Fi status feedback: slow blue blink (connecting), solid green (connected), fast red blink (disconnected), and a brief weather-condition colour pulse on each weather update.
- **Wi-Fi Status Icon**: Header bar Wi-Fi symbol changes color in real time (green = connected, red = disconnected).

## Hardware Requirements

- **ESP32 Cheap Yellow Device (CYD)**: ESP32-2432S028R — 2.8″ 320×240 ILI9341 LCD with XPT2046 resistive touch.
- **Onboard Sensors**: LDR photoresistor (GPIO 34), Backlight PWM (GPIO 21), RGB LED (GPIO 4/16/17).
- Micro-USB cable for power and programming.

## Getting Started

### 1. Secrets Setup

Wi-Fi credentials live in a Git-ignored secrets file to prevent committing them.

1. Copy the template:
   ```bash
   task init
   ```
   *(Or manually: `cp config/secrets.h.example config/secrets.h`)*

2. Edit `config/secrets.h`:
   ```cpp
   #define WIFI_SSID     "Your_WiFi_Network"
   #define WIFI_PASSWORD "Your_WiFi_Password"
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

## Settings Tab Reference

All settings below are configured by touch on the device and saved to flash:

| Setting | Description |
| :--- | :--- |
| **Unit (C/F)** | Toggle between Celsius and Fahrenheit. |
| **Auto Light** | Enable/disable LDR-driven automatic backlight control. |
| **Brightness** | Manual backlight level slider (disabled when Auto Light is on). |
| **Theme** | Catppuccin flavor selector — Mocha / Macchiato / Frappé / Latte. Full UI redraws on change. |
| **Timezone** | GMT offset (– / + buttons, range –12 to +14). |

---

## Development

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
