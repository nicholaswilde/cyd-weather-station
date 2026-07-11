#!/usr/bin/env bash
# ==============================================================================
#
# flash.sh
# -------
# Downloads the latest release from GitHub to the /tmp directory,
# extracts the bin files to the /tmp directory, checks if esptool is
# installed, and flashes the device.
#
# Usage: ./flash_latest.sh <SERIAL_PORT
#
# @author Nicholas Wilde, 0xb299a622                                                        
# @date 07 Aug 2025  
# @version 0.1.0
#
# ==============================================================================

set -euo pipefail

# --- variables ---
GITHUB_REPO="nicholaswilde/cyd-weather-station"
SERIAL_PORT="${1:-/dev/ttyACM0}"

# --- Constants ---
readonly BLUE=$(tput setaf 4)
readonly RED=$(tput setaf 1)
readonly YELLOW=$(tput setaf 3)
readonly RESET=$(tput sgr0)

readonly SCRIPT_NAME=$(basename "$0")

# --- functions ---

# Logging function
function log() {
  local type="$1"
  local message="$2"
  local color="$RESET"

  case "$type" in
    INFO)
      color="$BLUE";;
    WARN)
      color="$YELLOW";;
    ERRO)
      color="$RED";;
  esac

  echo -e "${color}${type}${RESET}[$(date +'%Y-%m-%d %H:%M:%S')] ${message}"
}


# Checks if a command exists.
function commandExists() {
  command -v "$1" >/dev/null 2>&1
}

function check_dependencies() {
  # --- check for dependencies ---
  if ! commandExists curl || ! commandExists grep || ! commandExists unzip || ! commandExists esptool ; then
    log "ERRO" "Required dependencies (curl, grep, unzip, esptool) are not installed." >&2
    exit 1
  fi  
}

function download_release(){
  RELEASE=$(curl -fsSL https://api.github.com/repos/${GITHUB_REPO}/releases/latest | grep -o '"tag_name": *"[^"]*"' | cut -d '"' -f 4)
  log "INFO" "Latest release: ${RELEASE}"

  # --- get the latest release download URL ---
  log "INFO" "Fetching the latest release from ${GITHUB_REPO}..."
  LATEST_RELEASE_URL=$(curl -s "https://api.github.com/repos/${GITHUB_REPO}/releases/latest" |  grep "browser_download_url" | grep -o 'https://[^"]*')

  if [ -z "${LATEST_RELEASE_URL}" ]; then
    log "ERRO" "Could not find the latest release zip file." >&2
    exit 1
  fi

  # --- download and extract the release ---
  TMP_DIR=$(mktemp -d)
  log "INFO" "Downloading latest release from ${LATEST_RELEASE_URL}..."
  curl -sL "${LATEST_RELEASE_URL}" -o "${TMP_DIR}/latest_release.zip"
}

function extract_files() {
  log "INFO" "Extracting bin files to ${TMP_DIR}..."
  unzip -o "${TMP_DIR}/latest_release.zip" -d "${TMP_DIR}" "*.bin" &> /dev/null
}

function flash_device() {
  log "INFO" "Ready to flash the device on port ${SERIAL_PORT}."

  esptool \
    --chip esp32s3 \
    --port "${SERIAL_PORT}" \
    --baud 921600 \
    --before default-reset \
    --after hard-reset \
    write-flash \
      -z \
      --flash-mode dio \
      --flash-freq 80m \
      --flash-size 16MB \
      0x0000 "${TMP_DIR}/bootloader.bin" \
      0x8000 "${TMP_DIR}/partitions.bin" \
      0x10000 "${TMP_DIR}/firmware.bin"
}

# Downloads and flashes the latest release.
function main() {
  check_dependencies  
  download_release
  extract_files
  # find "${TMP_DIR}" -name "*.bin" -print
  flash_device
  log "INFO" "--- Flashing complete (simulation) ---"
}

main "$@"
