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

## 2. Core Software Stack
* **Language:** C/C++
* **Core Framework:** Arduino Framework (PlatformIO `framework = arduino`)
* **UI Library:** LVGL (Light and Versatile Graphics Library) - v8 (matching Random Nerd Tutorials / RuiSantos example code)
* **TFT & Touch Drivers:** TFT_eSPI or native LVGL display/touch interface (configured for CYD pinout)
* **Custom Fonts:** Erik Flowers' Weather Icons font (uncompressed 48px C font file containing custom weather glyphs)
* **Font Asset Tooling:** `lv_font_conv` (Node.js/npm) for generating custom LVGL font assets

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
