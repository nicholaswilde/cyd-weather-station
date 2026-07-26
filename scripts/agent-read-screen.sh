#!/usr/bin/env bash
set -e

# Tab map: 0=current, 1=forecast, 2=hourly, 3=settings

TAB=$1
if [ -z "$TAB" ]; then
    echo "Usage: $0 <current|forecast|hourly|settings>"
    exit 1
fi

case "$TAB" in
    current) TAB_IDX=0 ;;
    forecast) TAB_IDX=1 ;;
    hourly) TAB_IDX=2 ;;
    settings) TAB_IDX=3 ;;
    *) echo "Invalid tab: $TAB. Must be current, forecast, hourly, or settings."; exit 1 ;;
esac

# Source .env for CYD_DEVICE_IP
if [ -f .env ]; then
    export $(grep -v '^#' .env | xargs)
else
    echo "Error: .env file not found."
    exit 1
fi

if [ -z "$CYD_DEVICE_IP" ]; then
    echo "Error: CYD_DEVICE_IP not set in .env."
    exit 1
fi

echo "Switching tab to $TAB..."
curl -sS -m 5 -d "index=${TAB_IDX}" "http://${CYD_DEVICE_IP}/api/tab" > /dev/null || true

sleep 2

mkdir -p screenshots
OUT_FILE="screenshots/agent_${TAB}.bmp"

echo "Capturing screenshot..."
if ! curl -sS -f -m 15 "http://${CYD_DEVICE_IP}/screenshot" -o "${OUT_FILE}"; then
    echo "Error: Failed to capture screenshot."
    exit 1
fi

sleep 2

if command -v uv >/dev/null 2>&1 && [ -f "./scripts/convert-screenshots.py" ]; then
    echo "Converting to PNG..."
    uv run ./scripts/convert-screenshots.py > /dev/null
fi

PNG_FILE="screenshots/agent_${TAB}.png"
if [ -f "$PNG_FILE" ]; then
    echo "Success! Screenshot saved to $PNG_FILE"
else
    echo "Success! Screenshot saved to $OUT_FILE"
fi
