# Specification: LDR Auto-Backlight Dimming (`ldr_backlight_20260708`)

## Overview
Implement automatic display backlight brightness control based on the ambient room light. The Cheap Yellow Display (CYD) features an onboard Light Dependent Resistor (LDR) to sense ambient light and uses a transistor on the display backlight pin (TFT_BL) to allow brightness control via pulse-width modulation (PWM).

## Functional Requirements
1. **LDR Sensor Reading**:
   - Read ambient light level via analog input on LDR pin **GPIO 34**.
   - Raw ADC values should be periodically sampled (e.g., every 1 second).
2. **Backlight PWM Control**:
   - Control the screen backlight on **GPIO 21** using ESP32's LEDC peripheral.
   - Configure a 12-bit or 8-bit LEDC resolution with a suitable frequency (e.g., 5000Hz) to prevent visible screen flickering.
3. **Brightness Mapping & Smoothing**:
   - Map LDR ADC values (0–4095) to PWM duty cycles. High ambient light should yield higher display brightness; low ambient light should dim the screen.
   - Enforce a minimum brightness floor (e.g., 10%) so the display is never completely black.
   - Implement a moving average or low-pass filter to prevent rapid screen flickering or sudden brightness jumps when transient shadows pass over the sensor.
   - Make the brightness transition smooth over a configurable duration.
4. **Configuration Flag**:
   - Add a configuration flag `USE_LDR_AUTO_BACKLIGHT` (boolean/define) in [config.h](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/config/config.h) to compile/enable the feature. When disabled, the backlight should default to full or a standard static brightness.

## Non-Functional Requirements
- **Resource Constraints**: Sampling the LDR sensor must not block the main LVGL GUI tick execution.
- **Power Efficiency**: Active PWM control should operate within standard ESP32 limits.

## Acceptance Criteria
- [ ] The screen backlight changes brightness dynamically in response to ambient light changes when enabled.
- [ ] A compile-time configuration flag `USE_LDR_AUTO_BACKLIGHT` in [config.h](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/config/config.h) can enable/disable the feature.
- [ ] Brightness transitions are smooth and do not cause display flickering.
- [ ] The display remains readable (above minimum floor) in a pitch-black room.
- [ ] Unit tests are implemented to verify the mapping and smoothing/filtering logic under mock ADC conditions.
- [ ] The system compiles successfully and passes static checks (`task check`).

## Out of Scope
- Runtime manual configuration of LDR calibration or target curves via UI (this belongs in the settings screen track).
