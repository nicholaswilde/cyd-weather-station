# Implementation Plan: Wi-Fi Manager AP Captive Portal

## Phase 1: Extend SettingsManager for Wi-Fi Persistence [checkpoint: 455b60c]
- [x] Task: Add SSID and Password to SettingsManager [f45dcd2]
    - [x] Update `include/settings_manager.h` to declare getter/setter for SSID and Password [f45dcd2]
    - [x] Update `src/settings_manager.cpp` to initialize, save, and load SSID and password fields from Preferences (falling back to secrets.h if not set) [f45dcd2]
- [x] Task: Write Tests for SettingsManager Wi-Fi persistence [f45dcd2]
    - [x] Write unit tests in `tests/test_settings/test_settings.cpp` to verify getters, setters, and default fallbacks for SSID and password [f45dcd2]
    - [x] Run tests to verify they fail (Red Phase) [f45dcd2]
    - [x] Complete implementation to make the tests pass (Green Phase) [f45dcd2]
- [x] Task: Conductor - User Manual Verification 'Phase 1: Extend SettingsManager for Wi-Fi Persistence' (Protocol in workflow.md) [455b60c]

## Phase 2: Implement AP Mode and Captive Portal Web Server
- [ ] Task: Update WifiManager interface and state machine
    - [ ] Update `include/wifi_manager.h` to support AP mode states (e.g., `WIFI_STATE_AP_MODE`), timeout trackers, and DNS/Web Server pointers
    - [ ] Implement AP setup using `<hostname>-<mac>` format (e.g. `cyd-weather-station-1A2B`)
- [ ] Task: Implement Captive Portal Web Server and DNS Server
    - [ ] Set up DNSServer to redirect all requests to device IP (192.168.4.1)
    - [ ] Set up WebServer to serve HTML configuration page with Wi-Fi scan results and SSID/Password form fields
- [ ] Task: Write Tests for WifiManager State Transitions
    - [ ] Add unit tests in `tests/test_wifi_manager/test_wifi_manager.cpp` for AP mode timeouts and status checks
    - [ ] Run tests to verify failure (Red Phase)
    - [ ] Pass the tests (Green Phase)
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Implement AP Mode and Captive Portal Web Server' (Protocol in workflow.md)

## Phase 3: Integrate Form Submission and Lifecycle Loop
- [ ] Task: Handle Captive Portal credentials submission
    - [ ] Parse POST parameters on submission, save via `SettingsManager`, and schedule reboot
- [ ] Task: Update main loop to handle connection fallback
    - [ ] Modify `src/main.cpp` to trigger AP Mode if connection to AP is not achieved within 30 seconds on boot
    - [ ] Ensure UI/status LED reflects AP mode state
- [ ] Task: Verify End-to-End Integration
    - [ ] Compile and build project using `task build`
    - [ ] Verify test suite passes with `pio test -e native`
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Integrate Form Submission and Lifecycle Loop' (Protocol in workflow.md)
