# Implementation Plan - screensaver-mode_20260711

## Phase 1: Inactivity Tracking & Screen State [checkpoint: ccfe7ce]
- [x] Task: Create `ScreenSaverManager` or add touch timestamp monitoring in `main.cpp` (652bc79, 047e94e, f865fe3)
- [x] Task: Implement soft-dimming and soft-brightening functions in `BacklightManager` (652bc79, 047e94e, f865fe3)

## Phase 2: Screensaver UI Layout
- [x] Task: Create a minimalist drifting clock view in `src/ui.cpp` (a695deb)
- [x] Task: Integrate random coordinate offsets periodically to drift the text position (a695deb)
- [x] Task: Hook the LVGL input device touch callback to wake the screen on click (a695deb)
- [x] Task: Conductor - User Manual Verification 'Phase 1 & 2' (Protocol in workflow.md) (a695deb)
