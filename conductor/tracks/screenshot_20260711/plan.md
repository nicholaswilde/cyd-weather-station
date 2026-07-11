# Implementation Plan - screenshot_20260711

## Phase 1: Settings Preference & Screenshot Core Logic [checkpoint: c05a382]
- [x] Task: Update `SettingsManager` to support screenshot server enabled setting. (4920bf2)
    - [x] Add `_screenshotServerEnabled` member variable (default to true).
    - [x] Implement `getScreenshotServerEnabled()` and `setScreenshotServerEnabled(bool enabled)`.
    - [x] Load and save `"scr_srv"` preference key.
- [x] Task: Create `ScreenshotManager` class. (e2971bd)
    - [x] Define helper functions to construct 54-byte BMP header for width/height.
    - [x] Implement `captureScreenToBMP` to allocate LVGL screenshot buffer and handle color space conversion (RGB565 to 24-bit BGR).
- [x] Task: Write unit tests for BMP formatting. (e2971bd)
    - [x] Verify that BMP header fields (file size, offset, width, height, color depth) are constructed correctly.
- [x] Task: Conductor - User Manual Verification 'Phase 1: Settings Preference & Screenshot Core Logic' (Protocol in workflow.md)

## Phase 2: Remote Screenshot Web Server Integration [checkpoint: 54f6689]
- [x] Task: Modify `WifiManager` to start WebServer in STA mode. (926635a)
    - [x] Create and start `_webServer` on port 80 when connected to Wi-Fi if `getScreenshotServerEnabled()` is true.
    - [x] Periodically call `_webServer->handleClient()` in `WifiManager::update()` during STA connected state.
- [x] Task: Implement `/screenshot` HTTP route handler. (926635a)
    - [x] Register `/screenshot` route on `_webServer`.
    - [x] Write headers and stream the BMP image directly to the client socket row-by-row to save memory.
- [x] Task: Write unit tests to check web server handlers and endpoint routing. (926635a)
- [x] Task: Conductor - User Manual Verification 'Phase 2: Remote Screenshot Web Server Integration' (Protocol in workflow.md)

## Phase 3: Physical BOOT Button Trigger & SD Card Storage [checkpoint: ]
- [x] Task: Set up physical BOOT button input handler. (966d76a)
    - [x] Define `BOOT_BUTTON_PIN` as GPIO 0.
    - [x] Initialize the pin as `INPUT_PULLUP` during setup.
- [x] Task: Implement button polling and SD write sequence in loop. (966d76a)
    - [x] In `loop()`, poll the button state and debounce presses.
    - [x] On press, generate a timestamped filename `/screenshot_YYYYMMDD_HHMMSS.bmp` using NTP time.
    - [x] Temporarily mount the SD card (if not already mounted), write the BMP header and pixel data row-by-row, then restore the previous SD mount state.
- [x] Task: Write unit tests for file name formatting and SD storage writing mocks. (966d76a)
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Physical BOOT Button Trigger & SD Card Storage' (Protocol in workflow.md)

## Phase 4: UI Toggle Control [checkpoint: 591890a]
- [x] Task: Add "Screenshot Server" toggle switch in Settings UI screen. (591890a)
    - [x] Add the toggle switch widget and align it in the settings toggles column.
    - [x] Add callback to update the preferences.
- [x] Task: Handle dynamic server starting/stopping. (591890a)
    - [x] If toggled off, immediately stop the WebServer listener.
    - [x] If toggled on and Wi-Fi is connected, start the WebServer listener.
- [ ] Task: Conductor - User Manual Verification 'Phase 4: UI Toggle Control' (Protocol in workflow.md)
