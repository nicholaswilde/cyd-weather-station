# Implementation Plan - screensaver-mode_20260711

## Phase 1: Inactivity Tracking & Screen State [checkpoint: dd0840b]
- [x] Task: Create `ScreenSaverManager` or add touch timestamp monitoring in `main.cpp` (652bc79, 047e94e)
- [x] Task: Implement soft-dimming and soft-brightening functions in `BacklightManager` (652bc79, 047e94e)

## Phase 2: Screensaver UI Layout
- [ ] Task: Create a minimalist drifting clock view in `src/ui.cpp`
- [ ] Task: Integrate random coordinate offsets periodically to drift the text position
- [ ] Task: Hook the LVGL input device touch callback to wake the screen on click
- [ ] Task: Conductor - User Manual Verification 'Phase 1 & 2' (Protocol in workflow.md)
