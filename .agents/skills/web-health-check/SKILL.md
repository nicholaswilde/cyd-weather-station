---
name: web-health-check
description: Skill to automate web server health checks and HTML validation on the live CYD device.
---

# Web Server Health & UI Validation Skill

This skill provides an automated bash script to test the responsiveness and stability of the CYD Weather Station's built-in web server.

## Prerequisites

1. The device must be powered on and connected to the same local network as your machine.
2. The `.env` file at the root of the project must contain the correct IP address in the `CYD_DEVICE_IP` variable.

## Usage

To run the web health check, execute the script from the repository root:

```bash
bash .agents/skills/web-health-check/web_health.sh
```

### What it does

1. Parses the `.env` file for the `CYD_DEVICE_IP`.
2. Iterates through a predefined list of essential web endpoints:
   - `/` (Root landing/React UI)
   - `/settings` (Settings HTML UI)
   - `/api/config` (JSON API)
   - `/api/tab?index=0` (Remote UI control)
   - `/api/orientation?val=1` (Remote UI control)
   - `/update` (OTA update page)
3. Issues a `curl` request to each endpoint with a 5-second timeout.
4. Verifies that the server returns a valid HTTP code (like `200 OK` or `403 Forbidden` if APIs are intentionally disabled by the user settings).
5. If the request times out or the connection is dropped (which typically indicates an ESP32 Out-Of-Memory crash or panic), the script marks the endpoint as a failure.

## Agent Guidelines

- Use this script when verifying that your changes to the `web_server.cpp` or `wifi_manager.cpp` logic did not introduce memory leaks or crashes that occur when endpoints are loaded.
- Execute this script using `ctx_execute` or `run_command`.
