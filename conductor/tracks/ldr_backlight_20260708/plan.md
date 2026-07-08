# Implementation Plan - LDR Auto-Backlight Dimming (`ldr_backlight_20260708`)

## Phase 1: PWM Backlight Control & Calibration [checkpoint: 2673f29]
- [x] Task: Design and test Backlight Control interface (5ad4e0e)
    - [x] Create backlight controller header and implementation files `include/backlight_manager.h` and `src/backlight_manager.cpp`
    - [x] Define `BacklightManager` class handling LEDC configuration and duty cycle adjustments
    - [x] Create unit tests in `tests/test_backlight` to verify duty cycle calculations and constraints
    - [x] Implement the core `BacklightManager` functions
    - [x] Verify unit tests pass successfully
- [x] Task: Conductor - User Manual Verification 'Phase 1: PWM Backlight Control & Calibration' (Protocol in workflow.md)

## Phase 2: LDR Sensor Integration & Smoothing
- [ ] Task: Add sensor reading and filtering logic
    - [ ] Add `USE_LDR_AUTO_BACKLIGHT` configuration flag to [config/config.h](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/config/config.h)
    - [ ] Extend `BacklightManager` to accept raw LDR analog readings
    - [ ] Write unit tests for exponential moving average (EMA) smoothing algorithm
    - [ ] Implement the EMA filter logic
    - [ ] Integrate backlight manager into [main.cpp](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/src/main.cpp) setup and loop (sampling every 1 second), wrapped inside conditional checks for the configuration flag
    - [ ] Verify test coverage and run lint checks via `task check`
- [ ] Task: Conductor - User Manual Verification 'Phase 2: LDR Sensor Integration & Smoothing' (Protocol in workflow.md)
