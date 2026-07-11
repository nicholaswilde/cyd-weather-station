# Specification: sd-offline-cache_20260711

## Overview
Implement an offline cache system on the SD card. When weather updates are retrieved successfully, write a serialized JSON representation of the `WeatherData` struct to `/weather_cache.json`. If the device boots without Wi-Fi, restore the UI elements from this file instead of presenting blank screens.

## Functional Requirements
1. **Cache Serialization**: Convert the `WeatherData` struct into a JSON string using `ArduinoJson` and write it to `/weather_cache.json` on the SD card.
2. **Offline Recovery Check**: If internet or Wi-Fi connection fails on boot, scan for `/weather_cache.json` on the SD card.
3. **Parse Cache**: Deserialize the cache file and load the values into the active `WeatherData` instance.
4. **UI Banner**: Display a subtle "Offline Cached Data" note or warning color badge on the status bar if the device is showing offline cache data.

## Acceptance Criteria
- Successful API weather updates write a valid JSON string to `/weather_cache.json`.
- Disconnecting Wi-Fi and rebooting the device loads the last cached temperature and forecast onto the screen.
- A warning message/icon indicates to the user that they are looking at offline cached data.
