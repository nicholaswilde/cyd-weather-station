# Specification: Interactive Settings Page (`interactive_settings_20260708`)

## Overview
Create a Settings screen (as a new tab in the LVGL Tabview) allowing runtime control over weather units (Celsius vs Fahrenheit), manual screen brightness, and the GMT timezone offset, using touch interaction.

## Functional Requirements
1. **Settings Tab Addition**:
   - Add a third tab to the main LVGL Tabview: **Settings**.
2. **Temperature Unit Selector**:
   - Add a switch or toggle button (e.g., `lv_switch` or `lv_btnmatrix`) on the Settings page to toggle between **Celsius (°C)** and **Fahrenheit (°F)**.
   - Changing the unit must update the weather client configuration, trigger a new fetch from the API, and immediately update the displayed units on the UI.
3. **Manual Brightness Slider**:
   - Add a slider (`lv_slider`) on the Settings page to manually control screen brightness (0% to 100%).
   - Moving the slider should adjust the PWM duty cycle of TFT_BL (GPIO 21) in real-time.
   - If the LDR Auto-Brightness track is also active, provide a checkbox/switch to toggle "Auto Brightness" on/off. When Auto is off, the manual slider takes precedence.
4. **Timezone Offset Configurator**:
   - Add buttons (e.g., "+" and "-" buttons) or a dropdown list to adjust the GMT timezone offset (e.g., -12 to +14 hours).
   - Changing the offset must trigger a call to Arduino's `configTime` with the updated offset, updating the time displayed in the header.

## Non-Functional Requirements
- **Immediate UI Updates**: Changes to settings (units, brightness, timezone) must yield immediate visual changes on screen.
- **Settings Persistence (Optional)**: If EEPROM or Preferences library is used, store user settings so they survive a reboot. (Prefer ESP32's `Preferences` library).

## Acceptance Criteria
- [ ] Settings tab is accessible and responsive to touch input.
- [ ] Toggling Celsius/Fahrenheit switches units on the weather client and pulls updated data from the API.
- [ ] Manual slider correctly changes TFT backlight brightness between 10% (minimum floor) and 100%.
- [ ] Timezone offset change updates the system time.
- [ ] Preferences are saved to ESP32 Flash and re-applied upon boot.
- [ ] Unit tests are written to verify the configuration model, preference loading/saving, and calculation logic.

## Out of Scope
- Wi-Fi SSID/password input on-screen (configured in secrets/code only).
