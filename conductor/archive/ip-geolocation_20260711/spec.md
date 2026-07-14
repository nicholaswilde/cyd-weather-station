# Specification: ip-geolocation_20260711

## Overview
Implement IP-based auto-geolocation as a fallback on boot. If the user hasn't configured a ZIP code or latitude/longitude in their settings, query `ip-api.com` or `ipapi.co` to retrieve coordinates and city name automatically.

## Functional Requirements
1. **Fallback Detection**: Detect if ZIP code and coordinates are empty or left at defaults.
2. **HTTP Geolocation Query**: Make an HTTP request to `http://ip-api.com/json/` when Wi-Fi is connected on boot.
3. **Parse Coordinates & City**: Parse latitude, longitude, and city name from the geolocate response.
4. **Pass to Weather Client**: Automatically populate `WeatherClient` parameters with these resolved values and update the weather tab.

## Acceptance Criteria
- Booting the device with empty settings triggers a geolocation HTTP fetch.
- On success, the city name, latitude, and longitude are logged and used to fetch local weather.
- Handles connection errors or rate-limits gracefully (falls back to configured manual defaults).
