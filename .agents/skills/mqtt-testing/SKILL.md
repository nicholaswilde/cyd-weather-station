---
name: mqtt-testing
description: Skill to automate testing of the CYD Weather Station's MQTT integration, including broker connectivity, status payloads, and commands.
---

# MQTT Integration Testing Skill

This skill provides an automated bash script to test the two-way MQTT communication between the CYD Weather Station and your local MQTT broker (like Mosquitto or Home Assistant).

## Prerequisites

1. The device must be powered on and connected to the local network.
2. The device must have MQTT enabled and correctly configured (broker IP, port, credentials).
3. The `.env` file at the root of the project must contain the correct IP address in the `CYD_DEVICE_IP` variable.
4. Your machine must have `jq` and `mosquitto-clients` (`mosquitto_pub` and `mosquitto_sub`) installed.

## Usage

To run the MQTT tests, execute the script from the repository root:

```bash
bash .agents/skills/mqtt-testing/test_mqtt.sh
```

### What it does

1. **Fetches Configuration**: Uses the `CYD_DEVICE_IP` to hit the `/api/config` JSON endpoint.
2. **Parses Credentials**: Uses `jq` to extract the live MQTT broker IP, port, username, password, and base topic directly from the device's current settings.
3. **Verifies Liveness**: Uses `mosquitto_sub` to subscribe to the `<base_topic>status` topic and ensure the device's retained status is `online`.
4. **Tests Command Handling**: 
   - Subscribes to the device's brightness state topic.
   - Publishes a new brightness value to the command topic using `mosquitto_pub`.
   - Listens for the device to acknowledge the command by updating its state topic, proving that the bi-directional Home Assistant integration is functioning correctly.

## Agent Guidelines

- Use this script when verifying changes made to `mqtt_manager.cpp` to ensure no functionality is broken.
- Execute this script using `ctx_execute` or `run_command`.
