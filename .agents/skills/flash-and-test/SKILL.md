---
name: flash-and-test
description: Skill to automate compiling and flashing firmware to a CYD board via USB serial, discovering its network IP from boot logs, updating .env, and running the live on-device validation test suite (web health, API, screen captures).
---

# Flash and Test Skill

This skill provides an automated workflow to compile firmware, flash it directly to a connected ESP32 Cheap Yellow Display (CYD) board over USB serial, monitor the boot output to discover its assigned DHCP IP address, synchronize `.env`, and execute the full live validation suite on the physical hardware.

## Prerequisites

1. The CYD device must be connected to the host computer via USB (e.g., `/dev/ttyUSB0` or `/dev/ttyACM0`).
2. The user account must have permissions to access the serial port (typically in the `dialout` or `uucp` group).
3. The device must be able to connect to the local Wi-Fi network.

## Usage

Run the automated script from the repository root:

```bash
# Flash the default environment (cyd_28r or PIO_ENV from .env) to auto-detected serial port and run all live tests:
bash .agents/skills/flash-and-test/flash_and_test.sh

# Flash a specific environment:
bash .agents/skills/flash-and-test/flash_and_test.sh cyd_28r
bash .agents/skills/flash-and-test/flash_and_test.sh cyd_35c

# Flash using a specific serial port:
bash .agents/skills/flash-and-test/flash_and_test.sh -p /dev/ttyUSB0 cyd_28r

# Run live tests on an already flashed and running device without rebuilding/uploading:
bash .agents/skills/flash-and-test/flash_and_test.sh --skip-flash

# Run host-native unit tests along with USB flash and live hardware tests:
bash .agents/skills/flash-and-test/flash_and_test.sh -u
```

### What It Does

1. **Auto-Detects Serial Port**: Automatically searches for `/dev/ttyUSB0`, `/dev/ttyACM0`, or detects via PlatformIO device list.
2. **Builds & Flashes Firmware**: Invokes `pio run -e <env> -t upload --upload-port <port>` to flash the device.
3. **Serial Boot IP Discovery**: Toggles RTS/DTR to cleanly reset the ESP32, captures serial boot logs, extracts the IP address from `[WiFi] Connected! IP address: <IP>`, and automatically updates `CYD_DEVICE_IP` in `.env`.
4. **Verifies Reachability**: Pings the web server on the discovered IP until the HTTP server becomes responsive.
5. **Runs Web Server Health Checks**: Executes the `web-health-check` skill (`web_health.sh`), validating HTTP 200 responses on `/`, `/settings`, `/api/config`, `/api/tab`, `/api/orientation`, and `/update`.
6. **Runs REST API Config Tests**: Executes the `test-api-config` skill (`test_api.sh`), validating GET and POST operations on `/api/config`.
7. **Captures UI Screen Verifications**: Enables the screenshot server and captures PNG screenshots of the `current`, `forecast`, and `hourly` tabs to `screenshots/` using `scripts/agent-read-screen.sh`.

## Agent Guidelines

- When prompted to flash the device via USB, test on physical hardware, or perform end-to-end device validation, execute this skill using `run_command`.
- After running the script, use `view_file` on `screenshots/agent_forecast.png` or `screenshots/agent_current.png` to visually inspect that the UI rendered as expected.
- For wireless network flashing without USB, refer to the `ota-updater` skill instead.
