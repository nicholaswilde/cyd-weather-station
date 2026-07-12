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

## How to capture screenshots

Run the automation script with the device's IP address:

```bash
./scripts/capture-screenshots.sh <DEVICE_IP>
```

This script will automatically transition the screen across orientations and tabs to generate:
- `screenshots/landscape_current.bmp`
- `screenshots/landscape_forecast.bmp`
- `screenshots/landscape_settings.bmp`
- `screenshots/portrait_current.bmp`
- `screenshots/portrait_forecast.bmp`
- `screenshots/portrait_settings.bmp`

## Updating README.md

After capturing new screenshots, reference them in the [README.md](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/README.md) file inside the screenshots section using HTML table or standard markdown image links.
