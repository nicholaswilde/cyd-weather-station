#!/bin/bash

# Determine path to .env file depending on where the script is run from
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

echo "=========================================="
echo "Testing API on device at IP: $DEVICE_IP"
echo "=========================================="

echo -e "\n--- 1. Testing GET /api/config ---"
GET_RESULT=$(curl -s "http://$DEVICE_IP/api/config")
if [ -z "$GET_RESULT" ]; then
    echo "FAILED: Could not connect to http://$DEVICE_IP/api/config"
    exit 1
fi

# Try to format with jq if available, else just echo
if command -v jq &> /dev/null; then
    echo "$GET_RESULT" | jq .
else
    echo "$GET_RESULT"
fi

echo -e "\n--- 2. Testing POST /api/config ---"
# We will toggle theme_flavor to a test value, you can change this payload as needed
TEST_PAYLOAD='{"theme_flavor": 1}'
echo "Sending payload: $TEST_PAYLOAD"
POST_RESULT=$(curl -s -X POST -H "Content-Type: application/json" -d "$TEST_PAYLOAD" "http://$DEVICE_IP/api/config")

if command -v jq &> /dev/null; then
    echo "$POST_RESULT" | jq .
else
    echo "$POST_RESULT"
fi

echo -e "\n--- 3. Testing Verification GET ---"
VERIFY_RESULT=$(curl -s "http://$DEVICE_IP/api/config")
echo "$VERIFY_RESULT" | grep -q '"theme_flavor":1'
if [ $? -eq 0 ]; then
    echo "SUCCESS: Configuration updated and successfully verified!"
else
    echo "FAILED: Configuration did not seem to update correctly."
    echo "Response: $VERIFY_RESULT"
    exit 1
fi

echo -e "\n=========================================="
echo "Tests Completed Successfully."
echo "=========================================="
