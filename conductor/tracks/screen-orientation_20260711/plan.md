# Implementation Plan - screen-orientation_20260711

## Phase 1: Settings Preference & State Management [checkpoint: e32d8c6]
- [x] Task: Update `SettingsManager` class (header and source) to support screen orientation persistence. (e32d8c6)
    - [x] Add `_screenOrientation` member variable (default to 1 / Landscape).
    - [x] Implement `getScreenOrientation()` and `setScreenOrientation(int orientation)`.
    - [x] Save and load the preference key `"screen_rot"` using ESP32 Preferences library.
- [x] Task: Write unit tests to verify preference storage of screen orientation. (e32d8c6)
    - [x] Add assertions in `tests/test_settings` to load, set, save, and reload different orientation values.
- [x] Task: Conductor - User Manual Verification 'Phase 1: Settings Preference & State Management' (Protocol in workflow.md)

## Phase 2: Dynamic Display & Touch Rotation Drivers [checkpoint: 799c7cb]
- [x] Task: Implement dynamic touchscreen mapping functions in `display.cpp`. (799c7cb)
    - [x] Update `my_touchpad_read()` to query the current orientation setting.
    - [x] Calculate coordinate mapping (X/Y axis swap, X inversion, Y inversion) for each of the 4 orientations (0, 1, 2, 3) to match visual pixels.
- [x] Task: Initialize rotation in boot sequence. (799c7cb)
    - [x] Retrieve screen orientation preference before calling display initialization.
    - [x] Pass the orientation value to `TFT_eSPI` (using `tft.setRotation()`) and touch setup.
- [x] Task: Write unit tests to verify coordinates mapping logic. (799c7cb)
    - [x] Implement mocks in test suite to verify coordinate conversion outputs for all four rotations.
- [x] Task: Conductor - User Manual Verification 'Phase 2: Dynamic Display & Touch Rotation Drivers' (Protocol in workflow.md)

## Phase 3: Dynamic UI Layout Adaptation [checkpoint: 19ecafa]
- [x] Task: Refactor layout generation to support dynamic dimensions. (19ecafa)
    - [x] Use `lv_disp_get_hor_res(NULL)` and `lv_disp_get_ver_res(NULL)` instead of hardcoded 320/240 dimensions.
    - [x] Implement a layout adapter function that adjusts container flows (flex column for portrait, flex row for landscape).
- [x] Task: Update the Settings and Forecast screen elements to wrap/stack cleanly. (19ecafa)
    - [x] Reposition Settings column containers so they fit in 240 width (portrait).
    - [x] Rearrange multi-day forecast lists or cards to fit portrait flow.
- [x] Task: Conductor - User Manual Verification 'Phase 3: Dynamic UI Layout Adaptation' (Protocol in workflow.md)

## Phase 4: Orientation Settings Dropdown UI & Runtime Refresh [checkpoint: 3fe33a9]
- [x] Task: Add the "Orientation" dropdown in Settings panel. (3fe33a9)
    - [x] Add a dropdown widget with options: `Landscape`, `Portrait`, `Landscape Rev`, `Portrait Rev`.
    - [x] Set current value based on settings.
- [x] Task: Implement runtime rotation event handling. (3fe33a9)
    - [x] Create `orientation_dropdown_event_cb` callback.
    - [x] Update display rotation (`tft.setRotation`), touch controller rotation, and update preference.
    - [x] Trigger an LVGL screen size change event and call layout adapter to refresh UI layouts without rebooting.
- [x] Task: Conductor - User Manual Verification 'Phase 4: Orientation Settings Dropdown UI & Runtime Refresh' (Protocol in workflow.md)
