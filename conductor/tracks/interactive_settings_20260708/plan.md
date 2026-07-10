# Implementation Plan - Interactive Settings Page (`interactive_settings_20260708`)

## Phase 1: Settings Data Model & Preferences Persistence [checkpoint: ba7e852]
- [x] Task: Create settings configuration manager (6486035)
    - [x] Create `include/settings_manager.h` and `src/settings_manager.cpp`
    - [x] Define the configuration data structures (units, manual/auto brightness state, brightness level, timezone offset)
    - [x] Implement saving and loading configurations using ESP32's `Preferences` library
    - [x] Create unit tests in `tests/test_settings` to verify loading, saving, and default recovery logic (with mocked flash preferences if running on native)
    - [x] Verify unit tests pass successfully
- [x] Task: Conductor - User Manual Verification 'Phase 1: Settings Data Model & Preferences Persistence' (Protocol in workflow.md) (ba7e852)

## Phase 2: UI Controls and State Integration
- [x] Task: Implement settings controls in LVGL UI (daec044)
    - [x] Add the third tab "Settings" to the LVGL Tabview in `src/ui.cpp`
    - [x] Add LVGL widgets: Switch for Temperature Unit, Slider for Brightness, Switch for Auto-Brightness, and Buttons/Spinbox for Timezone
    - [x] Implement callback functions for each control widget to update `SettingsManager` in real-time
    - [x] Integrate configuration callbacks with the main application loop in [main.cpp](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/src/main.cpp) (applying brightness settings to LEDC, calling `configTime` for timezone, and refetching weather for unit changes)
    - [x] Verify test suite runs successfully and passes lint checks via `task check`
- [ ] Task: Conductor - User Manual Verification 'Phase 2: UI Controls and State Integration' (Protocol in workflow.md)
