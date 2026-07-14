# Implementation Plan - ip-geolocation_20260711

## Phase 1: Client & Parsing Logic
- [x] Task: Write tests for geolocation response parsing in `test_weather_client` (a369693)
- [x] Task: Create `fetchIpLocation` method inside `WeatherClient` (a369693)
- [x] Task: Implement HTTP JSON parser for `ip-api.com` structure (a369693)

## Phase 2: Integration
- [ ] Task: Trigger fallback geolocation lookup in `main.cpp` if settings are empty
- [ ] Task: Update local coordinates on success and invoke weather refetch
- [ ] Task: Conductor - User Manual Verification 'Phase 1 & 2' (Protocol in workflow.md)
