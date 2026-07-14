# Implementation Plan - Boot Button Double Action

This plan details the steps to implement and verify the new Boot button behavior using a test-driven approach (TDD).

## Phase 1: Logic & Tests (TDD) [checkpoint: 75a7e52]
- [x] Task: Create `ButtonManager` class and unit tests (3e43a89)
    - [x] Create test file `tests/test_button/test_button.cpp` to verify debouncing, single press, and long press transitions.
    - [x] Define `ButtonManager` class interface in `include/button_manager.h`.
    - [x] Ensure that native tests fail initially (Red Phase).
- [x] Task: Implement `ButtonManager` class logic (9521b4e)
    - [x] Write implementation in `src/button_manager.cpp`.
    - [x] Detect single press on release (< 2s).
    - [x] Detect long press immediately upon reaching the 2s threshold.
    - [x] Verify that all native unit tests pass (Green Phase).
- [x] Task: Conductor - User Manual Verification 'Phase 1: Logic & Tests (TDD)' (Protocol in workflow.md)

## Phase 2: Action Bindings & UI Feedback
- [x] Task: Implement transient UI messages (f724b75)
    - [x] Implement `showUIStatusMessage(const char*)` in `src/ui.cpp` and `include/ui.h` using a timed `lv_obj_del_delayed` overlay.
- [x] Task: Integrate button actions in main loop (4e92033)
    - [x] Instantiate `ButtonManager` in `src/main.cpp`.
    - [x] On single press: trigger weather refresh and show "Refreshing weather..." UI message.
    - [x] On long press: capture screenshot and show "Screenshot saved!" UI message.
- [x] Task: Implement screensaver override (4e92033)
    - [x] Check `screensaver.isActive()` when button press begins.
    - [x] If screensaver is active, wake it and ignore/discard the button action.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Action Bindings & UI Feedback' (Protocol in workflow.md)
