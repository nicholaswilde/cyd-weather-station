# Specification: Implement basic weather station UI skeleton and Wi-Fi connection

## 1. Overview
The goal of this track is to scaffold the PlatformIO project, configure the hardware drivers for the ESP32-2432S028R Cheap Yellow Display (CYD), establish a stable Wi-Fi connection, and render the initial LVGL UI skeleton. This forms the foundation for the subsequent weather data fetching and sensor integration tracks.

## 2. Hardware Specs & Driver Configuration
* **Display:** ILI9341 driver via SPI. Resolution: 320x240 (landscape orientation).
* **Touch Controller:** XPT2046 resistive touch controller via SPI.
* **Libraries:**
  - TFT_eSPI (configured for the specific pinout of the ESP32-2432S028R).
  - LVGL (Light and Versatile Graphics Library) - v8.x.

## 3. Functional Requirements

### 3.1 PlatformIO & Task Runner Setup
* Scaffold the PlatformIO environment with `platformio.ini` configured for `espressif32` platform and Arduino framework.
* Set up a `Taskfile.yml` using `go-task` to manage build, upload, testing, and linting tasks.
* Integrate `uv` for python tools needed by PlatformIO.

### 3.2 Hardware & LVGL Initialization
* Configure the TFT_eSPI driver pinouts to match the CYD hardware schematics.
* Initialize the display and calibrate/configure the touch input.
* Initialize LVGL and register the display buffer and input device driver.

### 3.3 Wi-Fi Connection Manager
* Create a Wi-Fi manager component that reads credentials from a local configuration or hardcoded definitions.
* Attempt connection to the local network on startup, indicating connection progress via Serial output and the UI.
* Keep connection active and implement basic retry/reconnect logic.

### 3.4 LVGL UI Skeleton
* Build a clean 320x240 landscape UI.
* **Header Bar:** Shows a Wi-Fi status indicator (connected icon or status text) and a mock time/date string.
* **Main Area:** Displays placeholder cards for current temperature, humidity, and a weather status description.
* **Touch Test:** Include a simple interactive button that updates a label or prints to Serial when touched, validating that touch input is correctly integrated.

## 4. Acceptance Criteria
1. **Compilation:** The project compiles successfully using PlatformIO CLI without errors.
2. **Wi-Fi connection:** The device connects to Wi-Fi on boot and outputs its IP address to the Serial monitor.
3. **Display Output:** The screen lights up and displays the LVGL layout (Header + Placeholder fields).
4. **Touch Input:** Touching the screen/button prints the expected touch coordinates and triggers the click handler.
