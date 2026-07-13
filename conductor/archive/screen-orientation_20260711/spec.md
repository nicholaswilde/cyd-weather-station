# Specification: screen-orientation_20260711

## Overview
Enable the screen orientation of the ESP32 Cheap Yellow Display (CYD) to be rotated dynamically at runtime to any of the 4 standard orientations:
- Landscape (default, rotation 1 / 90°)
- Landscape Inverted (rotation 3 / 270°)
- Portrait (rotation 2 / 180°)
- Portrait Inverted (rotation 0 / 0°)

This feature requires updating the Settings UI, display initialization, touch calibration logic, and layout adapters to properly resize and stack components on 240x320 vs 320x240 screens.

## Functional Requirements
1. **Orientation Setting**:
   - Add an "Orientation" dropdown to the Settings screen with four options:
     - Landscape (default / 90° rotation)
     - Portrait (180° rotation)
     - Landscape Rev (270° rotation)
     - Portrait Rev (0° rotation)
   - Persist the selected orientation setting in Preferences flash memory across reboot cycles.

2. **Display Rotation**:
   - Apply the selected rotation to TFT_eSPI (using tft.setRotation(...)) and LVGL display driver configuration.
   - Instantly update the display rotation when selected in the dropdown.

3. **Resistive Touch Calibration**:
   - Dynamically adjust the coordinate mapping in the touch read callback (my_touchpad_read) in display.cpp.
   - Ensure touch inputs align accurately with the visual interface in all four orientations by swapping X/Y and inverting axes based on the selected rotation state.

4. **Dynamic UI Adaptation**:
   - Dynamically adjust the LVGL UI layouts to fit the new dimensions:
     - 320x240 for Landscape/Landscape Rev.
     - 240x320 for Portrait/Portrait Rev.
   - Re-arrange main tabview layout elements: stack key widgets vertically in portrait, and position them side-by-side in landscape.

## Acceptance Criteria
- User can change the screen orientation dynamically via a dropdown in the Settings tab.
- The screen rotates immediately to the chosen orientation.
- The touch inputs align perfectly with touch buttons in all 4 orientations.
- The UI fits and is readable on the screen in both landscape (320x240) and portrait (240x320) orientations.
- The chosen orientation is saved to flash and restored correctly on device startup.
