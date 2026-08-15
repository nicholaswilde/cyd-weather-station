#!/bin/bash

# OTA (Over-The-Air) Firmware Updater Script
# Builds the firmware and uploads it directly to the CYD device via HTTP.

set -e

# Default environment
ENV_NAME="cyd_28r"

# Check for environment argument
if [ ! -z "$1" ]; then
    ENV_NAME="$1"
fi

echo "========================================================"
echo "🚀 Starting OTA Firmware Update for $ENV_NAME"
echo "========================================================"

# Determine path to .env file
ENV_FILE=".env"
if [ -f "../../.env" ]; then ENV_FILE="../../.env"; fi
if [ -f "../../../.env" ]; then ENV_FILE="../../../.env"; fi

if [ ! -f "$ENV_FILE" ]; then
    echo "Error: .env file not found at $ENV_FILE"
    exit 1
fi

# Extract the CYD_DEVICE_IP variable
DEVICE_IP=$(grep '^CYD_DEVICE_IP=' "$ENV_FILE" | cut -d '=' -f2 | tr -d '"' | tr -d "'" | tr -d '\r')

if [ -z "$DEVICE_IP" ]; then
    echo "Error: CYD_DEVICE_IP not found in $ENV_FILE"
    exit 1
fi

echo -e "\n⏳ [1/3] Building firmware ($ENV_NAME)..."
pio run -e "$ENV_NAME"

FIRMWARE_BIN=".pio/build/$ENV_NAME/firmware.bin"
if [ ! -f "$FIRMWARE_BIN" ]; then
    echo "Error: Firmware binary not found at $FIRMWARE_BIN"
    exit 1
fi

echo "✅ Build successful!"

echo -e "\n⏳ [2/3] Checking device connectivity ($DEVICE_IP)..."
if ! curl -s "http://$DEVICE_IP/" > /dev/null; then
    echo "FAILED: Cannot connect to device at http://$DEVICE_IP/"
    exit 1
fi
echo "✅ Device is reachable!"

echo -e "\n⏳ [3/3] Uploading firmware to http://$DEVICE_IP/update..."
# The ESP32 WebServer Update library expects a multipart/form-data upload.
UPLOAD_RESULT=$(curl -s -w "%{http_code}" -X POST -F "update=@$FIRMWARE_BIN" "http://$DEVICE_IP/update")

HTTP_CODE=$(echo "$UPLOAD_RESULT" | tail -n 1)
RESPONSE_BODY=$(echo "$UPLOAD_RESULT" | sed '$d')

if [ "$HTTP_CODE" = "200" ]; then
    echo "✅ Firmware successfully uploaded!"
    echo "The device should now be rebooting..."
else
    echo "FAILED: Firmware upload failed with HTTP code $HTTP_CODE"
    echo "Response: $RESPONSE_BODY"
    exit 1
fi

echo -e "\n========================================================"
echo "🎉 SUCCESS: OTA Update Completed!"
echo "========================================================"
