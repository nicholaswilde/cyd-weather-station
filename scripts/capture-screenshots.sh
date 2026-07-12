#!/usr/bin/env bash
################################################################################
#
# capture-screenshots.sh
# ----------------
# Automates taking screen captures of all screens in both orientations.
#
# @author Nicholas Wilde, 0xb299a622
# @date 11 Jul 2026
# @version 0.1.2
#
################################################################################

# Options
set -e
set -o pipefail

# Constants
readonly BLUE=$(tput setaf 4)
readonly RED=$(tput setaf 1)
readonly YELLOW=$(tput setaf 3)
readonly RESET=$(tput sgr0)

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
  if ! commandExists curl ; then
    log "ERRO" "Required dependency (curl) is not installed." >&2
    exit 1
  fi  
}

function set_orientation() {
  local ip="$1"
  local rot="$2"
  local rot_name="$3"
  local err_msg

  log "INFO" "Setting orientation to ${rot_name} (val=${rot})..."
  if ! err_msg=$(curl -sS -m 5 -d "val=${rot}" "http://${ip}/api/orientation" 2>&1); then
    log "ERRO" "Connection failed: ${err_msg}" >&2
    log "WARN" "Please ensure the Cheap Yellow Device is powered on, connected to the same network, and the Screenshot Server (Scr Srv) is turned ON in the Settings tab." >&2
    exit 1
  fi
  sleep 2.5
}

function capture_tab() {
  local ip="$1"
  local rot_name="$2"
  local tab="$3"
  local tab_name="$4"
  local out_file="screenshots/${rot_name}_${tab_name}.bmp"
  local err_msg

  log "INFO" "Setting tab to ${tab_name} (index=${tab})..."
  if ! err_msg=$(curl -sS -m 5 -d "index=${tab}" "http://${ip}/api/tab" 2>&1); then
    log "ERRO" "Failed to change tab: ${err_msg}" >&2
    exit 1
  fi
  sleep 1.5

  log "INFO" "Capturing screenshot to ${out_file}..."
  mkdir -p screenshots
  if err_msg=$(curl -sS -f -m 10 "http://${ip}/screenshot" -o "${out_file}" 2>&1); then
    log "INFO" "Successfully saved ${out_file}."
  else
    log "ERRO" "Screenshot capture failed: ${err_msg}" >&2
    exit 1
  fi
}

# Main function to orchestrate the script execution
function main() {
  check_dependencies

  local ip="$1"
  if [ -z "${ip}" ]; then
    log "ERRO" "Usage: ./scripts/capture-screenshots.sh <DEVICE_IP>" >&2
    exit 1
  fi

  log "INFO" "Starting screenshot capture process from device ${ip}..."

  # 1. Capture Landscape screens (rotation 1)
  set_orientation "${ip}" 1 "landscape"
  capture_tab "${ip}" "landscape" 0 "current"
  capture_tab "${ip}" "landscape" 1 "forecast"
  capture_tab "${ip}" "landscape" 2 "settings"

  # 2. Capture Portrait screens (rotation 2)
  set_orientation "${ip}" 2 "portrait"
  capture_tab "${ip}" "portrait" 0 "current"
  capture_tab "${ip}" "portrait" 1 "forecast"
  capture_tab "${ip}" "portrait" 2 "settings"

  # 3. Restore to default Landscape orientation
  log "INFO" "Restoring default Landscape orientation..."
  curl -sS -m 5 -d "val=1" "http://${ip}/api/orientation" > /dev/null || true
  curl -sS -m 5 -d "index=0" "http://${ip}/api/tab" > /dev/null || true

  log "INFO" "Screenshots automation process finished successfully."
}

# Call main to start the script
main "$@"
