#!/bin/bash

# MQTT Integration Testing Script
# Connects to the device's MQTT broker using config dynamically fetched 
# from the device, and verifies connectivity and command handling.

set -e

# Extract IP from .env
ENV_FILE=".env"
if [ -f "../../.env" ]; then ENV_FILE="../../.env"; fi
if [ -f "../../../.env" ]; then ENV_FILE="../../../.env"; fi

if [ ! -f "$ENV_FILE" ]; then
    echo "Error: .env file not found at $ENV_FILE"
    exit 1
fi

DEVICE_IP=$(grep '^CYD_DEVICE_IP=' "$ENV_FILE" | cut -d '=' -f2 | tr -d '"' | tr -d "'" | tr -d '\r')

if [ -z "$DEVICE_IP" ]; then
    echo "Error: CYD_DEVICE_IP not found in $ENV_FILE"
    exit 1
fi

# Check dependencies
if ! command -v jq &> /dev/null; then
    echo "Error: 'jq' is required to parse the JSON config. Please install jq."
    exit 1
fi

if ! command -v mosquitto_pub &> /dev/null || ! command -v mosquitto_sub &> /dev/null; then
    echo "Error: 'mosquitto-clients' (mosquitto_pub/sub) is required. Please install it."
    exit 1
fi

# Fetch MQTT config from the device via API
echo "Fetching MQTT config from http://$DEVICE_IP/api/config ..."
CONFIG=$(curl -s -m 5 "http://$DEVICE_IP/api/config" || echo "")

if [ -z "$CONFIG" ]; then
    echo "Error: Could not fetch config from device."
    exit 1
fi

MQTT_ENABLED=$(echo "$CONFIG" | jq -r '.mqtt_enabled')
if [ "$MQTT_ENABLED" != "true" ]; then
    echo "Error: MQTT is not enabled on the device. Please enable it first via the UI or API."
    exit 1
fi

MQTT_SERVER=$(echo "$CONFIG" | jq -r '.mqtt_server')
MQTT_PORT=$(echo "$CONFIG" | jq -r '.mqtt_port')
MQTT_USER=$(echo "$CONFIG" | jq -r '.mqtt_user')
MQTT_PASS=$(echo "$CONFIG" | jq -r '.mqtt_password')
MQTT_BASE=$(echo "$CONFIG" | jq -r '.mqtt_base_topic')

echo "========================================================"
echo "📡 Testing MQTT Integration"
echo "Broker: $MQTT_SERVER:$MQTT_PORT"
echo "Base Topic: $MQTT_BASE"
echo "========================================================"

# Setup mosquitto auth arguments
AUTH_ARGS=""
if [ "$MQTT_USER" != "null" ] && [ "$MQTT_USER" != "" ]; then
    AUTH_ARGS="-u $MQTT_USER -P $MQTT_PASS"
fi

echo -e "\n⏳ [1/2] Checking if device is online..."
# Wait for the online status message. If device is already online, we might miss the retained message 
# if not retained, but usually status is retained. Let's just ask the broker for the retained status.
STATUS=$(mosquitto_sub -h "$MQTT_SERVER" -p "$MQTT_PORT" $AUTH_ARGS -t "${MQTT_BASE}status" -C 1 -W 3 || echo "timeout")
if [ "$STATUS" = "online" ]; then
    echo "✅ Device is online!"
else
    echo "❌ Device status not 'online' (Got: '$STATUS'). Ensure it is connected to MQTT."
    exit 1
fi

echo -e "\n⏳ [2/2] Testing command topic (Brightness)..."
# We'll use 55 as a test brightness
TEST_BRIGHTNESS=55
echo "Publishing brightness '$TEST_BRIGHTNESS' to ${MQTT_BASE}command/brightness"

# Start a subscriber in the background to catch the state update
rm -f /tmp/mqtt_test_bright.txt
mosquitto_sub -h "$MQTT_SERVER" -p "$MQTT_PORT" $AUTH_ARGS -t "${MQTT_BASE}settings/brightness" -C 1 -W 5 > /tmp/mqtt_test_bright.txt &
SUB_PID=$!

sleep 1 # wait for subscriber to connect

# Publish the command
mosquitto_pub -h "$MQTT_SERVER" -p "$MQTT_PORT" $AUTH_ARGS -t "${MQTT_BASE}command/brightness" -m "$TEST_BRIGHTNESS"

# Wait for subscriber to finish
wait $SUB_PID || true
RESULT=$(cat /tmp/mqtt_test_bright.txt || echo "error")

if [ "$RESULT" = "$TEST_BRIGHTNESS" ]; then
    echo "✅ Success! Device acknowledged the brightness command via state topic."
else
    echo "❌ Failed to get matching brightness state update. Got: '$RESULT'"
    exit 1
fi

echo -e "\n========================================================"
echo "🎉 SUCCESS: MQTT Integration Tests Passed!"
echo "========================================================"
