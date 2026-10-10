#!/usr/bin/env bash

# flash_and_test.sh
# Automates compiling and flashing firmware to a CYD board via USB serial,
# capturing boot logs to extract the assigned device IP, updating .env,
# and running live on-device validation tests (web health, API, screenshots).

set -eo pipefail

ENV_NAME=""
PORT=""
SKIP_FLASH=false
SKIP_SCREENSHOTS=false
RUN_UNIT_TESTS=false

# Print usage
usage() {
    cat <<EOF
Usage: $0 [options] [environment]

Arguments:
  [environment]           Target PlatformIO environment (e.g., cyd_28r, cyd_35c).
                          Defaults to PIO_ENV from .env, or cyd_28r.

Options:
  -p, --port <port>       Serial upload port (e.g., /dev/ttyUSB0).
                          Defaults to auto-detecting connected USB serial devices.
  -s, --skip-flash        Skip firmware build/upload and run on-device tests directly.
  --skip-screenshots      Skip taking on-device UI screen captures.
  -u, --unit-tests        Also execute host-native unit tests (pio test -e native).
  -h, --help              Show this help message.

Examples:
  $0                      # Flash default env to auto-detected port and test
  $0 cyd_28r              # Flash cyd_28r to auto-detected port and test
  $0 -p /dev/ttyUSB0      # Flash to /dev/ttyUSB0 and test
  $0 --skip-flash         # Run on-device tests on already running device
EOF
    exit 0
}

# Parse options
while [[ $# -gt 0 ]]; do
    case "$1" in
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        -s|--skip-flash)
            SKIP_FLASH=true
            shift
            ;;
        --skip-screenshots)
            SKIP_SCREENSHOTS=true
            shift
            ;;
        -u|--unit-tests)
            RUN_UNIT_TESTS=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        -*)
            echo "Unknown option: $1"
            usage
            ;;
        *)
            if [ -z "$ENV_NAME" ]; then
                ENV_NAME="$1"
            fi
            shift
            ;;
    esac
done

# Locate .env file
ENV_FILE=".env"
if [ -f "../../.env" ]; then ENV_FILE="../../.env"; fi
if [ -f "../../../.env" ]; then ENV_FILE="../../../.env"; fi

# Determine default environment
if [ -z "$ENV_NAME" ] && [ -f "$ENV_FILE" ]; then
    ENV_NAME=$(grep '^PIO_ENV=' "$ENV_FILE" | cut -d '=' -f2 | tr -d '"' | tr -d "'" | tr -d '\r' || true)
fi
if [ -z "$ENV_NAME" ]; then
    ENV_NAME="cyd_28r"
fi

echo "========================================================"
echo "⚡ CYD Flash & Live Device Test Suite"
echo "========================================================"
echo "Target Environment : $ENV_NAME"
echo "Skip Flash         : $SKIP_FLASH"
echo "Skip Screenshots   : $SKIP_SCREENSHOTS"
echo "Run Unit Tests     : $RUN_UNIT_TESTS"

# 1. Optional Unit Tests
if [ "$RUN_UNIT_TESTS" = true ]; then
    echo -e "\n--- [Step 1] Running Native Unit Tests ---"
    pio test -e native
    echo "✅ All native unit tests passed!"
fi

# 2. Port Detection & Flashing
if [ "$SKIP_FLASH" = false ]; then
    if [ -z "$PORT" ]; then
        # Auto-detect serial port
        if [ -e "/dev/ttyUSB0" ]; then
            PORT="/dev/ttyUSB0"
        elif [ -e "/dev/ttyACM0" ]; then
            PORT="/dev/ttyACM0"
        else
            # Try finding via pio device list
            PORT=$(pio device list 2>/dev/null | grep -E '^/dev/tty(USB|ACM)' | head -n 1 || true)
        fi
    fi

    if [ -z "$PORT" ] || [ ! -e "$PORT" ]; then
        echo "❌ Error: Could not detect USB serial port. Specify manually with -p <port>."
        exit 1
    fi
    echo "Detected Serial Port: $PORT"

    echo -e "\n--- [Step 2] Building and Flashing Firmware ($ENV_NAME -> $PORT) ---"
    pio run -e "$ENV_NAME" -t upload --upload-port "$PORT"
    echo "✅ Firmware flashed successfully!"

    # 3. Serial Boot Log & IP Discovery
    echo -e "\n--- [Step 3] Monitoring Boot Log & Discovering Device IP ---"
    
    # Locate a python binary with pyserial available
    PYTHON_BIN=""
    for p in python3 /home/nicholas/.local/share/mise/installs/pipx-platformio/6.1.19/platformio/bin/python; do
        if [ -x "$p" ] && "$p" -c "import serial" 2>/dev/null; then
            PYTHON_BIN="$p"
            break
        fi
    done

    DETECTED_IP=""
    if [ -n "$PYTHON_BIN" ]; then
        DETECTED_IP=$("$PYTHON_BIN" -c "
import serial, time, re, sys
try:
    s = serial.Serial('$PORT', 115200, timeout=1)
    s.setDTR(False)
    s.setRTS(True)
    time.sleep(0.1)
    s.setRTS(False)
    start = time.time()
    while time.time() - start < 15:
        line = s.readline().decode('utf-8', errors='replace')
        if line:
            sys.stderr.write(line)
            m = re.search(r'Connected!\s+IP\s+address:\s+([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)', line)
            if m:
                print(m.group(1))
                break
except Exception as e:
    sys.stderr.write(f'Serial read error: {e}\n')
" 2>/dev/null || true)
    fi

    if [ -n "$DETECTED_IP" ]; then
        echo -e "\n🎯 Discovered Device IP from serial boot: $DETECTED_IP"
        if [ -f "$ENV_FILE" ]; then
            if grep -q '^CYD_DEVICE_IP=' "$ENV_FILE"; then
                sed -i "s/^CYD_DEVICE_IP=.*/CYD_DEVICE_IP=$DETECTED_IP/" "$ENV_FILE"
            else
                echo "CYD_DEVICE_IP=$DETECTED_IP" >> "$ENV_FILE"
            fi
            echo "Updated CYD_DEVICE_IP in $ENV_FILE"
        fi
    else
        echo "⚠️ Could not auto-detect IP from serial output within 15 seconds. Using existing .env."
    fi
fi

# Extract Device IP from .env
DEVICE_IP=""
if [ -f "$ENV_FILE" ]; then
    DEVICE_IP=$(grep '^CYD_DEVICE_IP=' "$ENV_FILE" | cut -d '=' -f2 | tr -d '"' | tr -d "'" | tr -d '\r' || true)
fi

if [ -z "$DEVICE_IP" ]; then
    echo "❌ Error: CYD_DEVICE_IP not found in $ENV_FILE"
    exit 1
fi

echo -e "\n--- [Step 4] Verifying Device Reachability ($DEVICE_IP) ---"
REACHABLE=false
for attempt in {1..6}; do
    if curl -s -m 3 "http://$DEVICE_IP/" > /dev/null 2>&1; then
        REACHABLE=true
        break
    fi
    echo "Waiting for device HTTP server to start (attempt $attempt/6)..."
    sleep 2
done

if [ "$REACHABLE" = false ]; then
    echo "❌ Error: Device at http://$DEVICE_IP/ is not reachable."
    exit 1
fi
echo "✅ Device is online and responsive at http://$DEVICE_IP/"

# 4. Web Health Check
echo -e "\n--- [Step 5] Running Web Server Health Check ---"
if [ -f ".agents/skills/web-health-check/web_health.sh" ]; then
    bash .agents/skills/web-health-check/web_health.sh
else
    echo "⚠️ web_health.sh script not found, skipping."
fi

# 5. REST API Config Test
echo -e "\n--- [Step 6] Running REST API Config Tests ---"
if [ -f ".agents/skills/test-api-config/test_api.sh" ]; then
    bash .agents/skills/test-api-config/test_api.sh
else
    echo "⚠️ test_api.sh script not found, skipping."
fi

# 6. Visual Screen Verification
if [ "$SKIP_SCREENSHOTS" = false ]; then
    echo -e "\n--- [Step 7] Capturing On-Device UI Screenshots ---"
    
    # Ensure screenshot server is enabled
    curl -s -X POST -H "Content-Type: application/json" \
         -d '{"screenshot_server_enabled": true}' \
         "http://$DEVICE_IP/api/config" > /dev/null 2>&1 || true

    if [ -x "./scripts/agent-read-screen.sh" ]; then
        for tab in current forecast hourly; do
            echo "Capturing $tab tab..."
            ./scripts/agent-read-screen.sh "$tab" || true
        done
        echo "✅ Screenshots captured to screenshots/ directory."
    else
        echo "⚠️ scripts/agent-read-screen.sh not found or not executable, skipping screenshots."
    fi
fi

echo -e "\n========================================================"
echo "🎉 SUCCESS: All device flashes and tests completed!"
echo "========================================================"
