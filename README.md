# CYD Weather Station

A beautiful, configurable real-time weather station and desk clock built for the **ESP32 Cheap Yellow Device (CYD)** (board model ESP32-2432S028R) utilizing the **LVGL v8** graphics library, **Open-Meteo Weather API**, and **Catppuccin Color Theme**.

## Features

- **Real-Time Weather Details**: Fetches current temperature, humidity, and condition status directly from the free Open-Meteo API (requires no API keys).
- **Location Resolution**:
  - **Coordinates**: Enter latitude and longitude coordinates.
  - **Zip Code Geocoding**: Enter a US Zip Code (e.g. `90210`); the station resolves coordinates on boot using the Open-Meteo Geocoding API.
- **Dynamic Weather Status Icons**: Displays a large (48px) custom weather glyph corresponding to the current weather condition (clear, cloudy, rainy, snowy, stormy) mapped directly from WMO weather codes. Uses Erik Flowers' Weather Icons font, dynamically colored using the Catppuccin flavor palette (e.g., sunny yellow, rainy blue, cloudy/snowy lavender, stormy mauve).
- **Auto-Brightness Control**: Automatically dims the screen backlight in low-light environments and brightens it in well-lit rooms using the onboard LDR light sensor (GPIO 34) and PWM (GPIO 21).
- **NTP Time Synchronization**: Connects to NTP servers on boot to keep a real-time header bar clock.
- **Flexible Unit Selection**: Toggle between `UNIT_METRIC` (°C) and `UNIT_IMPERIAL` (°F) dynamically formatting both API requests and screen readouts.
- **Catppuccin Style Theme**: Full color palette styling support with configuration switches for four theme flavors:
  - **Mocha** (Default Dark Theme)
  - **Macchiato** (Medium Dark Theme)
  - **Frappé** (Soft Dark Theme)
  - **Latte** (Light Theme)
- **Status Indicators**: Clean header bar title, real-time clock, and a dynamic color-changing Wi-Fi symbol showing connection status (Green = Connected, Amber = Connecting, Red = Disconnected).

## Hardware Requirements

- **ESP32 Cheap Yellow Device (CYD)**: ESP32-2432S028R (2.8" 320x240 LCD screen with resistive touch chip XPT2046).
- **Onboard Sensors**: LDR photoresistor (GPIO 34) and Backlight control (PWM GPIO 21).
- Micro-USB cable for power and programming.

## Getting Started

### 1. Secrets Setup
To prevent committing credentials to Git, Wi-Fi configuration resides in a Git-ignored secrets file.
1. Run the initialize task to copy the secrets template:
   ```bash
   task init
   ```
   *(Alternatively, run `cp config/secrets.h.example config/secrets.h`)*
2. Open `config/secrets.h` and add your Wi-Fi SSID and Password:
   ```cpp
   #define WIFI_SSID "Your_WiFi_Network"
   #define WIFI_PASSWORD "Your_WiFi_Password"
   ```

### 2. Configuration
Configure location details, unit settings, theme customization, and auto-backlight control in [config/config.h](config/config.h):

- **Location Settings**:
  ```cpp
  #define USE_ZIP_CODE true
  #define WEATHER_ZIP_CODE "90210" // Default Zip Code

  // Coordinates fallback if USE_ZIP_CODE is false
  #define WEATHER_API_LATITUDE "37.7749"
  #define WEATHER_API_LONGITUDE "-122.4194"
  ```
- **Unit System Selection**:
  ```cpp
  // Choices: UNIT_METRIC (Celsius), UNIT_IMPERIAL (Fahrenheit)
  #define UNIT_SYSTEM UNIT_IMPERIAL
  ```
- **Theme Selection**:
  ```cpp
  // Choices: CATPPUCCIN_MOCHA, CATPPUCCIN_MACCHIATO, CATPPUCCIN_FRAPPE, CATPPUCCIN_LATTE
  #define CATPPUCCIN_FLAVOR CATPPUCCIN_MOCHA
  ```
- **Auto Backlight Settings**:
  ```cpp
  // Set to true to enable automated light sensor backlight control (via LDR pin 34)
  #define USE_LDR_AUTO_BACKLIGHT true
  ```

---

## Development

This repository is optimized using PlatformIO Core and includes support for both ESP32 hardware builds and native desktop mock testing.

### Custom Weather Icon Font
The weather icons are powered by Erik Flowers' Weather Icons font. The C font file `src/weather_icons_48.c` is generated from the TTF font inside `assets/fonts/` using `lv_font_conv`.
To regenerate or customize the font glyph range:
1. Ensure Node.js is installed, then set up the build dependencies:
   ```bash
   task setup:node
   ```
2. Generate the font:
   ```bash
   task font:generate
   ```

### Commands reference (via Taskfile or PlatformIO CLI):

| Action | Task Command | PlatformIO CLI Equivalent | Description |
| :--- | :--- | :--- | :--- |
| **Initialize** | `task init` | `cp config/secrets.h.example config/secrets.h` | Copies the secrets template to active config. |
| **Build** | `task build` | `pio run` | Compiles the ESP32 firmware binary. |
| **Upload** | `task upload` | `pio run --target upload` | Uploads the binary to the connected CYD board. |
| **Monitor** | `task monitor` | `pio device monitor` | Opens the Serial Monitor at 115200 baud. |
| **Native Tests**| `task test` | `pio test -e native` | Runs local mock tests on the host environment. |
| **Lint Check** | `task check` | `pio check` | Executes fast static code analysis (`cppcheck`). |
| **Setup Node** | `task setup:node` | `npm install` | Installs node tools (`lv_font_conv`) for font conversion. |
| **Generate Font** | `task font:generate` | `npm run font:generate` | Rebuilds the weather icons C source from the TTF file. |
| **Clean** | `task clean` | `pio run --target clean` | Deletes build output and temporary compilation files. |

## :balance_scale: License

[Apache License 2.0](LICENSE)

## :writing_hand: Author

This project was started in 2026 by [Nicholas Wilde](https://github.com/nicholaswilde/).

