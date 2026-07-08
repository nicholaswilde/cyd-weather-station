# Implementation Plan - LDR Auto-Backlight Dimming (`ldr_backlight_20260708`)

## Phase 1: PWM Backlight Control & Calibration [checkpoint: 2673f29]
- [x] Task: Design and test Backlight Control interface (5ad4e0e)
    - [x] Create backlight controller header and implementation files `include/backlight_manager.h` and `src/backlight_manager.cpp`
    - [x] Define `BacklightManager` class handling LEDC configuration and duty cycle adjustments
    - [x] Create unit tests in `tests/test_backlight` to verify duty cycle calculations and constraints
    - [x] Implement the core `BacklightManager` functions
    - [x] Verify unit tests pass successfully
- [x] Task: Conductor - User Manual Verification 'Phase 1: PWM Backlight Control & Calibration' (Protocol in workflow.md)

## Phase 2: LDR Sensor Integration & Smoothing [checkpoint: 5cc4a1e]
- [x] Task: Add sensor reading and filtering logic (e26b1fa)
    - [x] Add `USE_LDR_AUTO_BACKLIGHT` configuration flag to [config/config.h](file:///home/nicholas/git/nicholaswilde/config/config.h)
    - [x] Extend `BacklightManager` to accept raw LDR analog readings
    - [x] Write unit tests for exponential moving average (EMA) smoothing algorithm
    - [x] Implement the EMA filter logic
    - [x] Integrate backlight manager into [main.cpp](file:///home/nicholas/git/nicholaswilde/src/main.cpp) setup and loop (sampling every 1 second), wrapped inside conditional checks for the configuration flag
    - [x] Verify test coverage and run lint checks via `task check`
- [x] Task: Conductor - User Manual Verification 'Phase 2: LDR Sensor Integration & Smoothing' (Protocol in workflow.md)
