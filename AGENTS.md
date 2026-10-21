# Project Rules & Guidelines

## Codebase Lookup Preference
- When referring to the cyd-weather-station project itself, always look at the local repository first at `/home/nicholas/git/nicholaswilde/cyd-weather-station/` before searching elsewhere.
- Before performing any internet/web search for display, touchscreen, or hardware solutions for this ESP32 Cheap Yellow Display (CYD) project, **always search the local photo frame repository first** at `/home/nicholas/git/nicholaswilde/cyd-photo-frame/` for working reference code.
- Additionally, reference the local repository at `/home/nicholas/git/witnessmenow/ESP32-Cheap-Yellow-Display` for other features specific to the CYD.
- Reference `/home/nicholas/git/BruceDevices/firmware` for pinouts and hardware configurations for other CYD displays (such as `CYD-2432S028`, `CYD-2USB`, `CYD-2432W328C`, `CYD-2432W328R`/`S024R`, `CYD-3248S035R`/`C`, and `nm-cyd-c5`, excluding Elecrow devices).

## Build and Test Commands
- Build firmware: `pio run -e cyd_28r` or `pio run -e cyd_35c`
- Run host-native tests: `pio test -e native`

## RTK Command Guidelines
- **Git Operations**: Prefix `git` commands with `rtk` (e.g., `rtk git status`, `rtk git diff`, `rtk git log`, `rtk git commit`, `rtk git push`).
- **GitHub CLI**: Prefix `gh` commands with `rtk` (e.g., `rtk gh issue list | cat`, `rtk gh pr status | cat`). Always pipe `gh` commands to `cat` to bypass interactive pagers.
- **File & Directory Inspection**: Use `rtk ls`, `rtk tree`, `rtk find`, or `rtk read` when listing or reading files to get token-optimized output.
- **Searching**: Use `rtk grep` or `rtk rg` for line search pattern matching.
- **Build & Test Outputs**: Use `rtk err` or `rtk test` when running build/test commands to filter output to errors/failures only (e.g. `rtk test pio test -e native`).

## What To Do Next
- When asked "what to do next" (or similar), **always check the remote repository issues first** using `gh`:
  ```bash
  rtk gh issue list | cat
  ```

## Issue Creation
- When asked to create an issue, use your best guess to determine if it is a new feature or a bug fix.
- Prefix the issue title with `[feat]: <description>` or `[bug]: <description>`.
- Add the `enhancement` or `bug` label to the issue accordingly using the `--label` flag with the `gh` command.

## Issue Resolution & Testing
- When addressing or fixing GitHub issues, **always** create corresponding regression tests along with the issue implementations or fixes to prevent future regressions.

## Adding Settings Checklist
- Whenever adding a new setting or configuration option to the application, ensure it is consistently added across all integration points:
  1. **Default Config / Secrets**: Add default definitions to `include/config.h` or `include/secrets.h.example`.
  2. **Settings Web Page**: Add the input field, label, and form submission handler in `src/web_server.cpp` (or web settings page).
  3. **On-Device Settings UI**: Add the UI control (toggle, slider, dropdown) in `src/ui.cpp` and support bi-directional synchronization in `ui_sync_toggles()`.
  4. **MQTT Integration**: Add Home Assistant auto-discovery entity payload, state topic publishing, and command subscription/handling in `src/mqtt_manager.cpp` and `src/main.cpp`.
  5. **REST API Endpoint & Docs**: Add reading and updating logic for the new setting in `/api/config` GET/POST handlers (in `src/wifi_manager.cpp`) and update the example JSON response in `README.md`.

## Home Assistant MQTT Discovery Rules
- **Component Strictness**: Home Assistant is extremely strict with MQTT auto-discovery payloads. For example, if you define a `number` component without a `device_class`, it will fail silently if you provide an incompatible `unit_of_measurement` (such as `%`).
- **Validation Guidelines**: When making changes to MQTT Discovery payloads, be sure to omit `unit_of_measurement` unless it is explicitly paired with a documented HA `device_class` that requires it.
- **Testing**: Use the `.agents/skills/mqtt-testing/test_mqtt.sh` script to capture and validate that the discovery payloads successfully publish and do not violate Home Assistant constraints.

## Testing cyd_28c and cyd_28c_inv Variants
- The user does **not** have physical access to the JC2432W328C board (https://www.amazon.com/dp/B0D5H84RDB).
- When developing or testing for the `cyd_28c` and `cyd_28c_inv` environments, do not attempt to flash the device directly.
- Instead, always use the `package-binaries` skill to generate firmware ZIP files (partitions.bin, firmware.bin, bootloader.bin) so the user can send them to a tester.

## Context-Mode MCP Integration
This project uses `context-mode` for token-optimized command execution, file inspection, and searching.
- **Rules Reference**: See [`.agents/rules/context-mode-rules.md`](.agents/rules/context-mode-rules.md).

## Serena Semantic Code Navigation & LSP
This project uses Serena for semantic C/C++ and Python code navigation, AST symbol inspection, and LSP diagnostics.
- **Rules Reference**: See [`.agents/rules/serena-rules.md`](.agents/rules/serena-rules.md) for tool mappings, refactoring protocols, and CodeGraph boundaries.
- **Project Configuration**: Stored in `.serena/project.yml`.
- **Compilation Database**: Clangd uses `compile_commands.json` (generated via `pio run -t compiledb`).
- **Health Check**: Run `serena project health-check .` to verify language server connectivity and symbol lookup.
- **Index Codebase**: Run `serena project index .` to rebuild the symbol cache.

## Design System & Visual Guidelines
This project follows the DESIGN.md format specification (https://github.com/google-labs-code/design.md) for its visual identity, Catppuccin color tokens, typography scales, touch targets, and LVGL UI components.
- **Design System Reference**: See [`.agents/design.md`](.agents/design.md).
- **Validation**: Validate design system adherence using `designmd lint .agents/design.md`.

## Product Guide & Features

### Overview
The **ESP32 CYD LVGL Weather Station** is an interactive, touch-screen-enabled weather station app designed for the **ESP32-2432S028R** (Cheap Yellow Display / CYD) hardware. It fetches real-time weather data and forecasts from the free, open-source **Open-Meteo API** (or OpenWeatherMap) and displays it on the CYD screen using the **LVGL (Light and Versatile Graphics Library)** for a modern, responsive, and visually appealing user interface.

### Target Hardware
- **Display:** ESP32-2432S028R Cheap Yellow Display (CYD) 2.8" TFT touchscreen (typically ILI9341 driver + XPT2046 resistive touch controller), 2.8" capacitive (JC2432W328C), and 3.5" capacitive (ESP32-3248S035C).
- **Core:** ESP32 dual-core processor with integrated Wi-Fi.

### Key Features
1. **Real-time Weather Display:** Displays current weather conditions including temperature, humidity, wind speed, and precipitation.
2. **Forecast View:** Multi-day and hourly forecast screens to plan ahead.
3. **LVGL Touch UI:** A polished, touch-interactive interface with widgets, smooth animations, and visual clarity suited for small displays.
4. **Wi-Fi Connectivity:** Automated Wi-Fi connection with status indicators, reconnect logic, and an Access Point (AP) captive portal fallback configuration portal on timeout (30 seconds) allowing runtime Wi-Fi credentials setup.
5. **Open-Meteo Integration:** Automated periodic JSON API requests to fetch weather data without requiring an API key.
6. **Auto-Brightness Control:** Automatically dims the screen backlight in low-light environments and brightens it in well-lit rooms using the onboard LDR light sensor.
7. **Dynamic Weather Status Icons:** Displays a large (48px) weather icon corresponding to the current weather condition (clear, cloudy, rainy, snowy, stormy) next to the weather reading, dynamically colored using the Catppuccin color scheme.
8. **RGB LED Status Indicator:** Utilizes the onboard active-low RGB LED (GPIO 4/16/17) to provide secondary visual feedback about Wi-Fi connection states (slow blue blink for connecting, solid green for connected, fast red blink for disconnected, slow purple blink for AP mode) and brief weather status pulse notifications.
9. **Interactive Settings Screen:** Adds a Settings tab to the Tabview allowing runtime configuration of independent weather units, themes, manual screen brightness, Auto Light brightness mode, and timezone settings via touch controls, persisting settings across boots.
10. **SD Weather History & Sensor Data Logging:** Periodically logs hourly weather and system variables (temperature, humidity, wind speed, wind direction, weather code) alongside synchronized NTP timestamps to a CSV format file (`weather_history.csv`) on the root of the microSD card, with boot-time mounting and robust error handling.
11. **Wireless OTA Firmware Updates:** Supports Over-The-Air (OTA) firmware updates via a web browser (`http://192.168.4.1/update` or local IP `/update`), utilizing a Catppuccin-themed HTML upload portal with progress feedback and automatic reboot.
12. **Inactivity Sleep & Screen-Saver:** Auto-dims the screen backlight after configured inactivity and displays a floating digital clock overlay that drifts periodically to prevent screen pixel burn-in. Wakes instantly on any screen tap without triggering underlying widgets.
13. **Multi-Action Physical Button:** Re-purposes the onboard BOOT button (GPIO 0) for multiple actions: a single quick press triggers an immediate weather update, and a long press (>= 2 seconds) takes a screenshot saved to the SD card.
14. **SD Offline Cache Recovery:** Periodically saves weather data to microSD card (`/weather_cache.json`) in JSON format. On boot, if Wi-Fi connection fails or is unavailable, the device restores the cached weather data, updates the UI, and displays an offline status badge.
15. **IP-Based Auto-Location:** Automatically queries an IP geolocation API (`ip-api.com`) to resolve coordinates and city name on boot if the user has not configured manual coordinates or a ZIP code.

## Product Guidelines (UI/UX, Performance, Reliability)

### 1. User Interface (UI) & User Experience (UX) Design
* **Resolution Constraint:** All screens must fit a fixed 320x240 pixel portrait/landscape screen (ESP32-2432S028R default is landscape 320x240; also supports 320x480 for 3.5" boards).
* **Typography:**
  - Font sizes must be large enough to read from a distance of 1–2 meters (desk display).
  - Use clear, sans-serif fonts compatible with LVGL (e.g., standard Montserrat fonts built into LVGL).
* **Touch Targets:** Interactive elements (buttons, sliders, tabs) must have a minimum touch area of 40x40 pixels to facilitate easy control via resistive touchscreen.
* **Layout and Navigation:**
  - Keep the screen layout clean and uncluttered.
  - Use simple tab views or swipe gestures to navigate between the "Current Weather", "Forecast", "Hourly", and "Settings" views.
  - Provide immediate visual feedback for touch actions (e.g., button pressed state).

### 2. Technical and Performance Guidelines
* **Resource Optimization:**
  - Minimize unnecessary screen refreshes. Redraw only the UI components whose values have changed.
  - Compile custom icons or images as static C arrays to optimize flash/RAM memory usage.
* **Network & API Polling:**
  - Poll the Open-Meteo API at reasonable intervals (e.g., every 15–30 minutes) to avoid rate limits and save network bandwidth.
  - Perform HTTP requests asynchronously (if supported) or handle them in a way that doesn't block the main LVGL GUI loop or cause noticeable UI freezing.

### 3. Reliability & Error Recovery
* **Network Status Indication:** Always show a clear Wi-Fi connection indicator (signal icon) on the screen.
* **Graceful Degradation:**
  - If Wi-Fi is disconnected, show a visual warning. Do not crash the application.
  - If an API request fails, show the last successfully retrieved weather data along with an offline / stale data indicator.
* **Sensor Integration:** Design the data model to allow local sensors (e.g., DHT22/SHT40) to easily supplement external weather data.

## Technology Stack & Hardware Architecture

### 1. Hardware Target
* **Microcontroller:** ESP32-2432S028R (Cheap Yellow Display / CYD)
  - Processor: ESP32 dual-core Xtensa 32-bit LX6
  - Display: 2.8" TFT resistive touch screen (320x240) / 2.8" capacitive / 3.5" capacitive
  - Graphics Controller: ILI9341 / ST7789 / ST7796
  - Touch Controller: XPT2046 / CST816 / GT911
  - Ambient Light Sensor: Onboard LDR photoresistor (GPIO 34)
  - Backlight Control: PWM via LEDC (GPIO 21 or GPIO 27 depending on board)
  - Status Indicator: Onboard active-low RGB LED (Red: GPIO 4, Green: GPIO 16, Blue: GPIO 17)
  - Storage: Onboard microSD card slot (CS: GPIO 5, SCK: GPIO 18, MISO: GPIO 19, MOSI: GPIO 23, utilizing VSPI bus)

### 2. Core Software Stack
* **Language:** C/C++
* **Core Framework:** Arduino Framework (PlatformIO `framework = arduino`)
* **UI Library:** LVGL (Light and Versatile Graphics Library) - v8
* **TFT & Touch Drivers:** TFT_eSPI or native LVGL display/touch interface (configured for CYD pinout)
* **Custom Fonts:** Erik Flowers' Weather Icons font (uncompressed 48px and 24px C font files containing custom weather glyphs)
* **Font Asset Tooling:** `lv_font_conv` (Node.js/npm) for generating custom LVGL font assets
* **Preferences Storage:** ESP32 `Preferences` library (for saving runtime configurations in Flash memory across reboot cycles)
* **Captive Portal Servers:** `DNSServer` and `WebServer` standard ESP32 libraries (for routing and serving the setup configuration page in AP mode)
* **Storage & File System:** `SD` and `FS` standard ESP32 libraries (for card mounting and appending weather data records in CSV format)
* **OTA Firmware Updates:** ESP32 `Update` library (for chunked flashing of firmware binary files wirelessly via the WebServer)
* **Screen-Saver & Sleep Mode:** Custom `ScreenSaverManager` class integrated with BacklightManager fading and LVGL input touch callback hooks.

### 3. Build, Execution, and Tooling
* **Build System:** PlatformIO (Core CLI), managed via `platformio.ini` (Targets: Espressif 32 platform)
* **Task Runner:** `go-task` (managed via `Taskfile.yml` in the root directory)
* **Testing & Linting:**
  - Testing: PlatformIO built-in test runner (`pio test`) with Unity test framework
  - Linting: PlatformIO static code analysis (`pio check`) using `cppcheck`

### 4. Network and APIs
* **Weather Service:** Open-Meteo API (primary, free, no key required) and OpenWeatherMap (optional with API key)
* **HTTP Client:** HTTPClient (standard Arduino library)
* **JSON Parser:** ArduinoJson (for parsing API responses)

## Code Style Principles

### Readability
- Code should be easy to read and understand by humans.
- Avoid overly clever or obscure constructs.

### Consistency
- Follow existing patterns in the codebase.
- Maintain consistent formatting, naming, and structure.

### Simplicity
- Prefer simple solutions over complex ones.
- Break down complex problems into smaller, manageable parts.

### Maintainability
- Write code that is easy to modify and extend.
- Minimize dependencies and coupling.

### Documentation
- Document *why* something is done, not just *what*.
- Keep documentation up-to-date with code changes.



