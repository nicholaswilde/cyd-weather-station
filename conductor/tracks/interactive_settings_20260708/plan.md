# Implementation Plan - Interactive Settings Page (`interactive_settings_20260708`)

## Phase 1: Settings Data Model & Preferences Persistence
- [ ] Task: Create settings configuration manager
    - [ ] Create `include/settings_manager.h` and `src/settings_manager.cpp`
    - [ ] Define the configuration data structures (units, manual/auto brightness state, brightness level, timezone offset)
    - [ ] Implement saving and loading configurations using ESP32's `Preferences` library
    - [ ] Create unit tests in `tests/test_settings` to verify loading, saving, and default recovery logic (with mocked flash preferences if running on native)
    - [ ] Verify unit tests pass successfully
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Settings Data Model & Preferences Persistence' (Protocol in workflow.md)

## Phase 2: UI Controls and State Integration
- [ ] Task: Implement settings controls in LVGL UI
    - [ ] Add the third tab "Settings" to the LVGL Tabview in `src/ui.cpp`
    - [ ] Add LVGL widgets: Switch for Temperature Unit, Slider for Brightness, Switch for Auto-Brightness, and Buttons/Spinbox for Timezone
    - [ ] Implement callback functions for each control widget to update `SettingsManager` in real-time
    - [ ] Integrate configuration callbacks with the main application loop in [main.cpp](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/src/main.cpp) (applying brightness settings to LEDC, calling `configTime` for timezone, and refetching weather for unit changes)
    - [ ] Verify test suite runs successfully and passes lint checks via `task check`
- [ ] Task: Conductor - User Manual Verification 'Phase 2: UI Controls and State Integration' (Protocol in workflow.md)
