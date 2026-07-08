# CYD Weather Station

A beautiful, configurable real-time weather station and desk clock built for the **ESP32 Cheap Yellow Device (CYD)** (board model ESP32-2432S028R) utilizing the **LVGL v8** graphics library, **Open-Meteo Weather API**, and **Catppuccin Color Theme**.

## Features

- **Real-Time Weather Details**: Fetches current temperature, humidity, and condition status directly from the free Open-Meteo API (requires no API keys).
- **Location Resolution**:
  - **Coordinates**: Enter latitude and longitude coordinates.
  - **Zip Code Geocoding**: Enter a US Zip Code (e.g. `90210`); the station resolves coordinates on boot using the Open-Meteo Geocoding API.
- **NTP Time Synchronization**: Connects to NTP servers on boot to keep a real-time header bar clock.
- **Flexible Unit Selection**: Toggle between `UNIT_METRIC` (°C) and `UNIT_IMPERIAL` (°F) dynamically formatting both API requests and screen readouts.
- **Catppuccin Style Theme**: Full color palette styling support with configuration switches for four theme flavors:
  - **Mocha** (Default Dark Theme)
  - **Macchiato** (Medium Dark Theme)
  - **Frappé** (Soft Dark Theme)
  - **Latte** (Light Theme)
- **Status Indicators**: Clean header bar title, real-time clock, and a dynamic color-changing Wi-Fi symbol showing connection status (Green = Connected, Amber = Connecting, Red = Disconnected).
- **Interactive Touch Test**: Built-in touch validation button linked to the resistive screen controller.

## Hardware Requirements

- **ESP32 Cheap Yellow Device (CYD)**: ESP32-2432S028R (2.8" 320x240 LCD screen with resistive touch chip XPT2046).
- Micro-USB cable for power and programming.

## Getting Started

### 1. Secrets Setup
To prevent committing credentials to Git, Wi-Fi configuration resides in a Git-ignored secrets file.
1. Copy the secrets template:
   ```bash
   cp config/secrets.h.example config/secrets.h
   ```
2. Open `config/secrets.h` and add your Wi-Fi SSID and Password:
   ```cpp
   #define WIFI_SSID "Your_WiFi_Network"
   #define WIFI_PASSWORD "Your_WiFi_Password"
   ```

### 2. Configuration
Configure location details, unit settings, and theme customization in [config/config.h](config/config.h):

- **Location Settings**:
  ```cpp
  #define USE_ZIP_CODE true
  #define WEATHER_ZIP_CODE "90210" // Default Zip Code
  ```
- **Unit System Selection**:
  ```cpp
  // Choices: UNIT_METRIC (Celsius), UNIT_IMPERIAL (Fahrenheit)
  #define UNIT_SYSTEM UNIT_METRIC
  ```
- **Theme Selection**:
  ```cpp
  // Choices: CATPPUCCIN_MOCHA, CATPPUCCIN_MACCHIATO, CATPPUCCIN_FRAPPE, CATPPUCCIN_LATTE
  #define CATPPUCCIN_FLAVOR CATPPUCCIN_MOCHA
  ```

---

## Development

This repository is optimized using PlatformIO Core and includes support for both ESP32 hardware builds and native desktop mock testing.

### Commands reference (via Taskfile or PlatformIO CLI):

| Action | Task Command | PlatformIO CLI Equivalent | Description |
| :--- | :--- | :--- | :--- |
| **Build** | `task build` | `pio run` | Compiles the ESP32 firmware binary. |
| **Upload** | `task upload` | `pio run --target upload` | Uploads the binary to the connected CYD board. |
| **Monitor** | `task monitor` | `pio device monitor` | Opens the Serial Monitor at 115200 baud. |
| **Native Tests**| `task test` | `pio test -e native` | Runs local mock tests on the host environment. |
| **Lint Check** | `task check` | `pio check` | Executes fast static code analysis (`cppcheck`). |