# Specification: screensaver-mode_20260711

## Overview
Implement a low-brightness screen-saver mode. If no touch inputs are received for 5 minutes (or during scheduled night hours), dim the screen to 5% brightness and display a floating minimal clock. Tap the screen to wake instantly.

## Functional Requirements
1. **Inactivity Timer**: Maintain a timestamp of the last touch event. If `millis() - lastTouchTime > 300000`, trigger screen-saver.
2. **Backlight Dimming**: Dim the LEDC PWM backlight to 5% on activation.
3. **Drifting Clock UI**: Hide main tabs and show a single large digital clock that slowly drifts across the screen to prevent pixel burn-in.
4. **Wake Gesture**: Tap anywhere on the screen-saver to restore normal brightness (80% or LDR auto-light) and show the main weather tabs.

## Acceptance Criteria
- Screen dims and drifting clock appears after exactly 5 minutes of inactivity.
- Tapping anywhere wakes the screen immediately without lag.
- The clock changes position on screen periodically to prevent screen burn-in.
