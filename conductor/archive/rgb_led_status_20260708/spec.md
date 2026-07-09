# Specification: RGB LED Status Indicator (`rgb_led_status_20260708`)

## Overview
Utilize the onboard RGB LED of the Cheap Yellow Display (CYD) board to provide secondary visual feedback about the system status (WiFi connection phases) and current weather status.

## Functional Requirements
1. **LED Pin Configuration**:
   - Red LED: **GPIO 4** (Active-low, write `LOW` to turn on).
   - Green LED: **GPIO 16** (Active-low, write `LOW` to turn on).
   - Blue LED: **GPIO 17** (Active-low, write `LOW` to turn on).
2. **Wi-Fi Connection States**:
   - **Connecting**: Blinking Blue (slow blink, e.g., 500ms on, 500ms off).
   - **Connected**: Solid Green for 3 seconds, then turn off (to save power/prevent distraction).
   - **Disconnected / Error**: Blinking Red (fast blink, e.g., 200ms on, 200ms off).
3. **Weather State Visualization (Optional/Configurable)**:
   - When new weather data is fetched:
     - Clear/Sunny: Brief yellow pulse.
     - Rain/Snow: Brief blue pulse.
     - Thunderstorm/Extreme: Blinking Red alert (until acknowledged or next poll).
     - Overcast/Fog: Brief white pulse (R+G+B).

## Non-Functional Requirements
- **Non-Blocking Operation**: The blinking logic must use non-blocking timestamps (based on `millis()`) and not use `delay()`, ensuring the main LVGL thread runs smoothly.
- **Low Power Consumption**: Provide an option to turn off the LEDs or set a low brightness/pulse-only pattern to prevent distraction and save power.

## Acceptance Criteria
- [ ] LED blinking patterns match the connection states (connecting, connected, disconnected).
- [ ] LED controls are non-blocking and do not stutter the LVGL interface.
- [ ] The RGB LED pins are configured correctly as active-low.
- [ ] Unit tests are written to verify the state transitions and blink timer logic of the `LedManager`.
- [ ] Compilation is clean and lints pass (`task check`).

## Out of Scope
- Configurable LED brightness via the UI (belongs in the settings track).
