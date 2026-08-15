---
name: test-api-config
description: Skill to automate testing of the CYD Weather Station's /api/config endpoint using the IP address defined in the .env file.
---

# Test API Config Skill

This skill provides an automated way to test the JSON REST API endpoints on the live CYD Weather Station device.

## Prerequisites

1. The device must be flashed with the firmware that supports both `GET` and `POST` on `/api/config`.
2. The device must be connected to the local network.
3. The `.env` file at the root of the project must contain the correct IP address in the `CYD_DEVICE_IP` variable.

## Usage

To run the API tests, execute the `test_api.sh` script located in this skill folder from the root of the repository. Make sure the script is executable first:

```bash
chmod +x .agents/skills/test-api-config/test_api.sh
bash .agents/skills/test-api-config/test_api.sh
```

### What it does

1. Parses the `.env` file for `CYD_DEVICE_IP`.
2. Issues a `GET` request to `http://<IP>/api/config` and pretty-prints the JSON response using `jq`.
3. Issues a `POST` request to update a configuration variable (e.g., `theme_flavor`).
4. Issues a final `GET` request to verify the configuration change was successfully persisted on the device.

## Extending the tests

To add more configuration keys or test different endpoints, modify the `.agents/skills/test-api-config/test_api.sh` script. Make sure any new JSON structures use the correct types (e.g., integers vs strings as defined by `SettingsManager`).
