#!/bin/bash

# Web Server Health & UI Validation Script
# Hits various web endpoints on the CYD Weather Station to verify 
# they return valid HTTP responses and do not crash the device.

set -e

echo "========================================================"
echo "🌐 Starting Web Server Health Checks"
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

echo "Testing device at IP: $DEVICE_IP"
echo ""

# Array of endpoints to test
ENDPOINTS=(
    "/"
    "/settings"
    "/api/config"
    "/api/tab?index=0"
    "/api/orientation?val=1"
    "/update"
)

FAILS=0

for ENDPOINT in "${ENDPOINTS[@]}"; do
    echo -n "Hitting $ENDPOINT ... "
    
    # We use curl with a 5-second timeout and get the HTTP status code.
    # We allow redirects just in case.
    HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -m 5 "http://$DEVICE_IP$ENDPOINT" || echo "ERROR")
    
    if [ "$HTTP_CODE" = "ERROR" ] || [ "$HTTP_CODE" = "000" ]; then
        echo "❌ FAILED (Connection timeout, refused, or device crashed)"
        FAILS=$((FAILS + 1))
    elif [ "$HTTP_CODE" = "200" ]; then
        echo "✅ OK (200)"
    elif [ "$HTTP_CODE" = "403" ]; then
        echo "✅ OK (403 Forbidden - Endpoint is disabled in settings, but server is healthy)"
    elif [ "$HTTP_CODE" = "405" ]; then
        echo "✅ OK (405 Method Not Allowed - Server responded properly)"
    else
        echo "⚠️ WARNING (HTTP $HTTP_CODE)"
    fi
done

echo ""
echo "========================================================"
if [ $FAILS -eq 0 ]; then
    echo "🎉 SUCCESS: All web endpoints responded successfully!"
else
    echo "❌ FAILED: $FAILS endpoint(s) did not respond or crashed the device."
    exit 1
fi
echo "========================================================"
