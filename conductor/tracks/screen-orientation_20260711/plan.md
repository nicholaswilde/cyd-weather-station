# Implementation Plan - screen-orientation_20260711

## Phase 1: Settings Preference & State Management [checkpoint: e32d8c6]
- [x] Task: Update `SettingsManager` class (header and source) to support screen orientation persistence. (e32d8c6)
    - [x] Add `_screenOrientation` member variable (default to 1 / Landscape).
    - [x] Implement `getScreenOrientation()` and `setScreenOrientation(int orientation)`.
    - [x] Save and load the preference key `"screen_rot"` using ESP32 Preferences library.
- [x] Task: Write unit tests to verify preference storage of screen orientation. (e32d8c6)
    - [x] Add assertions in `tests/test_settings` to load, set, save, and reload different orientation values.
- [x] Task: Conductor - User Manual Verification 'Phase 1: Settings Preference & State Management' (Protocol in workflow.md)

## Phase 2: Dynamic Display & Touch Rotation Drivers [checkpoint: ]
- [ ] Task: Implement dynamic touchscreen mapping functions in `display.cpp`.
    - [ ] Update `my_touchpad_read()` to query the current orientation setting.
    - [ ] Calculate coordinate mapping (X/Y axis swap, X inversion, Y inversion) for each of the 4 orientations (0, 1, 2, 3) to match visual pixels.
- [ ] Task: Initialize rotation in boot sequence.
    - [ ] Retrieve screen orientation preference before calling display initialization.
    - [ ] Pass the orientation value to `TFT_eSPI` (using `tft.setRotation()`) and touch setup.
- [ ] Task: Write unit tests to verify coordinates mapping logic.
    - [ ] Implement mocks in test suite to verify coordinate conversion outputs for all four rotations.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Dynamic Display & Touch Rotation Drivers' (Protocol in workflow.md)

## Phase 3: Dynamic UI Layout Adaptation [checkpoint: ]
- [ ] Task: Refactor layout generation to support dynamic dimensions.
    - [ ] Use `lv_disp_get_hor_res(NULL)` and `lv_disp_get_ver_res(NULL)` instead of hardcoded 320/240 dimensions.
    - [ ] Implement a layout adapter function that adjusts container flows (flex column for portrait, flex row for landscape).
- [ ] Task: Update the Settings and Forecast screen elements to wrap/stack cleanly.
    - [ ] Reposition Settings column containers so they fit in 240 width (portrait).
    - [ ] Rearrange multi-day forecast lists or cards to fit portrait flow.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Dynamic UI Layout Adaptation' (Protocol in workflow.md)

## Phase 4: Orientation Settings Dropdown UI & Runtime Refresh [checkpoint: ]
- [ ] Task: Add the "Orientation" dropdown in Settings panel.
    - [ ] Add a dropdown widget with options: `Landscape`, `Portrait`, `Landscape Rev`, `Portrait Rev`.
    - [ ] Set current value based on settings.
- [ ] Task: Implement runtime rotation event handling.
    - [ ] Create `orientation_dropdown_event_cb` callback.
    - [ ] Update display rotation (`tft.setRotation`), touch controller rotation, and update preference.
    - [ ] Trigger an LVGL screen size change event and call layout adapter to refresh UI layouts without rebooting.
- [ ] Task: Conductor - User Manual Verification 'Phase 4: Orientation Settings Dropdown UI & Runtime Refresh' (Protocol in workflow.md)
