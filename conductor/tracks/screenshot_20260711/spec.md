# Specification: screenshot_20260711

## Overview
Implement a screenshot capture utility for the ESP32 Cheap Yellow Display (CYD) that allows capturing the active display content as a 24-bit uncompressed BMP image. 

The screenshots can be obtained in two ways:
1. **Remotely**: Via an HTTP GET request to `http://<cyd-ip>/screenshot`.
2. **Locally**: By pressing the physical BOOT button (GPIO 0) on the CYD board, which saves the screenshot directly to the root of the microSD card as a timestamped BMP file.

This feature is toggleable via the Settings tab to control whether the screenshot web server is active.

## Functional Requirements
1. **Screenshot Generation**:
   - Capture the current LVGL active screen.
   - Convert the 16-bit RGB565 screen buffer into a standard 24-bit uncompressed BMP format (with 54-byte BMP header).
   - Handle the rotation dynamically to ensure the image matches the screen's active orientation (Landscape or Portrait).

2. **HTTP Server Integration**:
   - When the screenshot server is enabled and the device is connected to Wi-Fi, run an HTTP GET endpoint at `/screenshot`.
   - The endpoint must stream the BMP image directly in the HTTP response using `image/bmp` content type.
   - Streaming must write row-by-row to prevent holding the entire 230 KB BMP file in memory.

3. **Physical Button Trigger & SD Card storage**:
   - Initialize the BOOT button (GPIO 0) with internal pull-up.
   - Detect a button press of the BOOT button.
   - When pressed, capture the screenshot and write it to `/screenshot_YYYYMMDD_HHMMSS.bmp` on the SD card (if present and mounted).
   - Use the active RTC/NTP synchronized time for the filename timestamp.

4. **Settings UI Integration**:
   - Add a "Screenshot Server" toggle in the Settings screen.
   - Persist the setting (enable/disable) in Preferences flash memory under the key `"scr_srv"`.
   - When toggled off, stop the HTTP server listener (or reject requests with 403 Forbidden).

## Acceptance Criteria
- Accessing `http://<cyd-ip>/screenshot` via a browser or curl command returns a valid 24-bit BMP image of the current screen.
- Pressing the physical BOOT button on the CYD creates a timestamped BMP file on the SD card.
- A toggle switch in the Settings UI correctly enables/disables the remote HTTP screenshot endpoint.
- The screenshot functionality handles both landscape (320x240) and portrait (240x320) orientations correctly.
