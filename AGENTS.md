# Project Rules & Guidelines

## Codebase Lookup Preference
- Before performing any internet/web search for display, touchscreen, or hardware solutions for this ESP32 Cheap Yellow Display (CYD) project, **always search the local photo frame repository first** at `/home/nicholas/git/nicholaswilde/cyd-photo-frame/` for working reference code.

## Build and Test Commands
- Build firmware: `pio run -e cyd_28r` or `pio run -e cyd_35c`
- Run host-native tests: `pio test -e native`

## RTK Command Guidelines
- **Git Operations**: Prefix `git` commands with `rtk` (e.g., `rtk git status`, `rtk git diff`, `rtk git log`, `rtk git commit`, `rtk git push`).
- **GitHub CLI**: Prefix `gh` commands with `rtk` (e.g., `rtk gh issue list | cat`, `rtk gh pr status | cat`). Always pipe `gh` commands to `cat` to bypass interactive pagers.
- **File & Directory Inspection**: Use `rtk ls`, `rtk tree`, `rtk find`, or `rtk read` when listing or reading files to get token-optimized output.
- **Searching**: Use `rtk grep` or `rtk rg` for line search pattern matching.
- **Build & Test Outputs**: Use `rtk err` or `rtk test` when running build/test commands to filter output to errors/failures only (e.g. `rtk test pio test -e native`).

## What To Do Next
- When asked "what to do next" (or similar), **always check the remote repository issues first** using `gh`:
  ```bash
  rtk gh issue list | cat
  ```

## Issue Creation
- When asked to create an issue, use your best guess to determine if it is a new feature or a bug fix.
- Prefix the issue title with `[feat]: <description>` or `[bug]: <description>`.
- Add the `enhancement` or `bug` label to the issue accordingly using the `--label` flag with the `gh` command.

## Issue Resolution & Testing
- When addressing or fixing GitHub issues, **always** create corresponding regression tests along with the issue implementations or fixes to prevent future regressions.

## Adding Settings Checklist
- Whenever adding a new setting or configuration option to the application, ensure it is consistently added across all integration points:
  1. **Default Config / Secrets**: Add default definitions to `include/config.h` or `include/secrets.h.example`.
  2. **Settings Web Page**: Add the input field, label, and form submission handler in `src/web_server.cpp` (or web settings page).
  3. **On-Device Settings UI**: Add the UI control (toggle, slider, dropdown) in `src/ui.cpp` and support bi-directional synchronization in `ui_sync_toggles()`.
  4. **MQTT Integration**: Add Home Assistant auto-discovery entity payload, state topic publishing, and command subscription/handling in `src/mqtt_manager.cpp` and `src/main.cpp`.
  5. **REST API Endpoint & Docs**: Add reading and updating logic for the new setting in `/api/config` GET/POST handlers (in `src/wifi_manager.cpp`) and update the example JSON response in `README.md`.

## Home Assistant MQTT Discovery Rules
- **Component Strictness**: Home Assistant is extremely strict with MQTT auto-discovery payloads. For example, if you define a `number` component without a `device_class`, it will fail silently if you provide an incompatible `unit_of_measurement` (such as `%`).
- **Validation Guidelines**: When making changes to MQTT Discovery payloads, be sure to omit `unit_of_measurement` unless it is explicitly paired with a documented HA `device_class` that requires it.
- **Testing**: Use the `.agents/skills/mqtt-testing/test_mqtt.sh` script to capture and validate that the discovery payloads successfully publish and do not violate Home Assistant constraints.
