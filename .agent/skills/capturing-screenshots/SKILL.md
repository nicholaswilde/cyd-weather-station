---
name: capturing-screenshots
description: Automates taking screen captures of all tab panels (Current, Forecast, Settings) in both orientations (Landscape, Portrait) from the CYD Weather Station device.
---

# Capturing Screenshots Skill

This skill provides the instructions and guidelines for an agent to capture, organize, and update screenshots of the CYD Weather Station.

## Prerequisites

- The Cheap Yellow Device (CYD) must be powered on, running the latest firmware, and connected to the same local network as the host.
- The web server must be running and accessible.
- The utility script [capture-screenshots.sh](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/scripts/capture-screenshots.sh) must be executable.
- Python 3 and the `uv` tool must be installed on the host.

## How to capture screenshots

Run the automation script with the device's IP address:

```bash
./scripts/capture-screenshots.sh <DEVICE_IP>
```

This script will automatically transition the screen across orientations and tabs to generate:
- `screenshots/landscape_current.png`
- `screenshots/landscape_forecast.png`
- `screenshots/landscape_hourly.png`
- `screenshots/landscape_settings.png`
- `screenshots/portrait_current.png`
- `screenshots/portrait_forecast.png`
- `screenshots/portrait_hourly.png`
- `screenshots/portrait_settings.png`

## Updating README.md

After capturing new screenshots, reference them in the [README.md](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/README.md) file inside the screenshots section using HTML table or standard markdown image links.
