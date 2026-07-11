# Specification: Wi-Fi Manager AP Captive Portal

## Overview
Implement an Access Point (AP) mode Captive Portal fallback for the ESP32 Cheap Yellow Device (CYD). If the device is unable to connect to the configured Wi-Fi network within 30 seconds, it will spin up a soft AP and host a web captive portal. This allows the user to connect to the device using their phone or computer, see a list of nearby networks, and input the SSID and password. These credentials will be stored in `Preferences` and persisted across reboots.

## Functional Requirements
1. **AP Mode Fallback**:
   - If Wi-Fi connection attempts in Station mode fail or take longer than 30 seconds on boot, switch the Wi-Fi mode to Access Point + Station (AP+STA) or AP.
   - Host a Soft AP with the SSID format: `cyd-weather-station-<mac_short>` where `<mac_short>` is the last 4 characters of the device's MAC address (e.g. `cyd-weather-station-1A2B`).
   - The AP should be open (no password).
2. **Captive Portal / Web Server**:
   - Host a lightweight DNS server to redirect all HTTP requests to the captive portal configuration page.
   - Host a web server presenting a clean, mobile-friendly configuration page.
   - The page must scan and list available Wi-Fi networks so the user can select one, or manually enter an SSID, and enter the password.
   - On submission, the device should save the credentials, show a success page, and reboot/retry connecting.
3. **SSID/Password Persistence**:
   - Save the SSID and password into the existing `Preferences` storage via `SettingsManager`.
   - On boot, if credentials exist in `Preferences`, use them. Otherwise, fall back to the default `WIFI_SSID` and `WIFI_PASSWORD` defined in `secrets.h`.
4. **Visual Indicator (Status LED)**:
   - Use the RGB status LED to indicate AP mode (e.g., slow blinking yellow or purple pulse).

## Non-Functional Requirements
- High reliability: The DNS redirection must successfully redirect common captive portal check URLs (e.g. apple, android captive portal checkers).
- Compact memory footprint to avoid OOM.
- Fast loading and scan time.

## Acceptance Criteria
- Device boots up, fails to connect to a non-existent SSID, and switches to AP mode after 30 seconds.
- Broadcasts `cyd-weather-station-XXXX` SSID.
- Connecting to the SSID launches a Captive Portal redirecting to the configuration web page.
- Select a network, enter password, submit, and confirm it saves to Preferences and successfully connects to that network on reboot.

## Out of Scope
- Dynamic tab interface inside the captive portal (keep it single-page).
- HTTPS support on the captive portal web server (not needed and too heavy).
