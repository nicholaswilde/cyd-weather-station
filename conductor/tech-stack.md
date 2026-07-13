# Technology Stack: ESP32 CYD LVGL Weather Station

## 1. Hardware Target
* **Microcontroller:** ESP32-2432S028R (Cheap Yellow Display / CYD)
  - Processor: ESP32 dual-core Xtensa 32-bit LX6
  - Display: 2.8" TFT resistive touch screen (320x240)
  - Graphics Controller: ILI9341
  - Touch Controller: XPT2046
  - Ambient Light Sensor: Onboard LDR photoresistor (GPIO 34)
  - Backlight Control: PWM via LEDC (GPIO 21)
  - Status Indicator: Onboard active-low RGB LED (Red: GPIO 4, Green: GPIO 16, Blue: GPIO 17)
  - Storage: Onboard microSD card slot (CS: GPIO 5, SCK: GPIO 18, MISO: GPIO 19, MOSI: GPIO 23, utilizing VSPI bus)

## 2. Core Software Stack
* **Language:** C/C++
* **Core Framework:** Arduino Framework (PlatformIO `framework = arduino`)
* **UI Library:** LVGL (Light and Versatile Graphics Library) - v8 (matching Random Nerd Tutorials / RuiSantos example code)
* **TFT & Touch Drivers:** TFT_eSPI or native LVGL display/touch interface (configured for CYD pinout)
* **Custom Fonts:** Erik Flowers' Weather Icons font (uncompressed 48px and 24px C font files containing custom weather glyphs)
* **Font Asset Tooling:** `lv_font_conv` (Node.js/npm) for generating custom LVGL font assets
* **Preferences Storage:** ESP32 `Preferences` library (for saving runtime configurations in Flash memory across reboot cycles)
* **Captive Portal Servers:** `DNSServer` and `WebServer` standard ESP32 libraries (for routing and serving the setup configuration page in AP mode)
* **Storage & File System:** `SD` and `FS` standard ESP32 libraries (for card mounting and appending weather data records in CSV format)
* **OTA Firmware Updates:** ESP32 `Update` library (for chunked flashing of firmware binary files wirelessly via the WebServer)



## 3. Build, Execution, and Tooling
* **Build System:** PlatformIO (Core CLI)
  - Managed via `platformio.ini`
  - Targets: Espressif 32 platform (`platform = espressif32`)
* **Task Runner:** `go-task` (managed via `Taskfile.yml` in the root directory)
* **Python Environment:** Global `pio` installation (relying on system-wide PlatformIO setup)
* **Testing & Linting:**
  - Testing: PlatformIO built-in test runner (`pio test`)
  - Linting: PlatformIO static code analysis (`pio check`) using `cppcheck` or `clang-tidy`

## 4. Network and APIs
* **Weather Service:** Open-Meteo API (free, open-source weather API, no API key required)
* **HTTP Client:** HTTPClient (standard Arduino library)
* **JSON Parser:** ArduinoJson (for parsing Open-Meteo responses)
