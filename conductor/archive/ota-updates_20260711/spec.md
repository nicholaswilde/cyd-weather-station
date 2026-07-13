# Specification: ota-updates_20260711

## Overview
Implement Over-The-Air (OTA) firmware updates via a web browser when the device is in AP mode or via a dedicated Settings option. This allows flashing the device wirelessly.

## Functional Requirements
1. **OTA Route**: The web server should listen on `/update` in AP Mode.
2. **Firmware Upload Form**: Serve a simple, beautiful Catppuccin-themed HTML page with a file upload selector.
3. **Upload Handling**: Utilize Arduino's `Update.h` framework to write the uploaded binary to the flash partition.
4. **Status & Reboot**: Show real-time progress of update success/failure, and restart the ESP32 on success.

## Acceptance Criteria
- Accessing `http://192.168.4.1/update` while connected to the AP loads the upload page.
- Uploading a valid compiled firmware binary restarts the device with the new code.
- Mismatched or corrupted binary files are rejected and report an error.
