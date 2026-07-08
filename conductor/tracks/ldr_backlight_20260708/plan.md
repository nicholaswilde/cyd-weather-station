# Implementation Plan - LDR Auto-Backlight Dimming (`ldr_backlight_20260708`)

## Phase 1: PWM Backlight Control & Calibration
- [ ] Task: Design and test Backlight Control interface
    - [ ] Create backlight controller header and implementation files `include/backlight_manager.h` and `src/backlight_manager.cpp`
    - [ ] Define `BacklightManager` class handling LEDC configuration and duty cycle adjustments
    - [ ] Create unit tests in `tests/test_backlight` to verify duty cycle calculations and constraints
    - [ ] Implement the core `BacklightManager` functions
    - [ ] Verify unit tests pass successfully
- [ ] Task: Conductor - User Manual Verification 'Phase 1: PWM Backlight Control & Calibration' (Protocol in workflow.md)

## Phase 2: LDR Sensor Integration & Smoothing
- [ ] Task: Add sensor reading and filtering logic
    - [ ] Add `USE_LDR_AUTO_BACKLIGHT` configuration flag to [config/config.h](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/config/config.h)
    - [ ] Extend `BacklightManager` to accept raw LDR analog readings
    - [ ] Write unit tests for exponential moving average (EMA) smoothing algorithm
    - [ ] Implement the EMA filter logic
    - [ ] Integrate backlight manager into [main.cpp](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/src/main.cpp) setup and loop (sampling every 1 second), wrapped inside conditional checks for the configuration flag
    - [ ] Verify test coverage and run lint checks via `task check`
- [ ] Task: Conductor - User Manual Verification 'Phase 2: LDR Sensor Integration & Smoothing' (Protocol in workflow.md)
