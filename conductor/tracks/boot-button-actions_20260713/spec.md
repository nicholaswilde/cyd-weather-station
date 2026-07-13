# Specification: Boot Button Double Action (Refresh & Screenshot)

## Overview
Currently, pressing the physical BOOT button (GPIO 0) on the CYD board triggers an immediate screenshot saved to the SD card. This track changes the behavior of the BOOT button to support two actions based on press duration:
1. **Single Quick Press**: Triggers a manual update/refresh of the weather data.
2. **Long Press (>= 2 seconds)**: Triggers a screen capture/screenshot saved to the SD card.

## Functional Requirements
1. **Button Debouncing and Duration Tracking**:
   - Detect state transitions on the physical BOOT button (GPIO 0, active-low).
   - Track duration of the press to distinguish between a quick push and a long press.
   - If the button is released in less than 2.0 seconds, register a "single quick press".
   - If the button is held for 2.0 seconds (2000 ms) or longer, trigger a "long press" action (can be triggered immediately once the 2s threshold is reached).
2. **Single Press Action (Refresh)**:
   - Request a weather update asynchronously.
   - Update the UI elements and cache when the weather data is fetched.
   - Display a brief on-screen status message (e.g. "Refreshing weather...").
3. **Long Press Action (Screenshot)**:
   - Capture the current screen and save to the SD card.
   - Display a brief on-screen status message (e.g. "Screenshot saved!").
4. **Screensaver Interaction**:
   - If the screensaver is active, any button press wakes the screen and ignores the refresh/screenshot action.

## Acceptance Criteria
- Pressing BOOT button for < 2s initiates weather update, shows "Refreshing weather..." on screen.
- Pressing and holding BOOT button for >= 2s takes a screenshot to SD card, shows "Screenshot saved!" on screen, and does not trigger refresh.
- Pressing BOOT button when screensaver is active wakes the device without triggering refresh or screenshot.

## Out of Scope
- Customizing button mappings via Settings UI.
