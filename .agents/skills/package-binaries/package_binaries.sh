#!/usr/bin/env bash
# ==============================================================================
# package_binaries.sh
# --------------------
# Compiles firmware for specified PlatformIO environments and creates ZIP archives
# containing partitions.bin, firmware.bin, and bootloader.bin for testing/flashing.
#
# Usage:
#   bash .agents/skills/package-binaries/package_binaries.sh [OPTIONS] [ENV1] [ENV2] ...
#
# Options:
#   -b, --bundle [ZIP_NAME]   Bundle all specified environments into a single ZIP archive.
#                             If ZIP_NAME is omitted, defaults to <first_env>_bundle.zip.
#   -o, --output-zip ZIP_NAME Output ZIP archive name for bundle mode.
#
# Defaults to: cyd_28c cyd_28c_inv (if no environments are passed)
# ==============================================================================

set -euo pipefail

# Output directory for release / test zip packages
OUTPUT_DIR="${OUTPUT_DIR:-dist}"
mkdir -p "${OUTPUT_DIR}"

# Helper to determine firmware version matching the GUI (APP_VERSION / extra_scripts.py)
get_firmware_version() {
  # 1. Prefer git describe (same command used by scripts/extra_scripts.py for APP_VERSION)
  if [ -d ".git" ] && command -v git &>/dev/null; then
    local ver
    ver=$(git describe --dirty --always --tags 2>/dev/null || true)
    if [ -n "$ver" ]; then
      echo "$ver"
      return 0
    fi
  fi

  # 2. Check generated include/version.h
  if [ -f "include/version.h" ]; then
    local ver
    ver=$(grep '#define APP_VERSION' include/version.h 2>/dev/null | sed -E 's/.*"([^"]+)".*/\1/')
    if [ -n "$ver" ] && [ "$ver" != "unknown" ]; then
      echo "$ver"
      return 0
    fi
  fi

  # 3. Fallback to version.txt
  if [ -f "version.txt" ]; then
    local ver
    ver=$(tr -d '[:space:]' < version.txt)
    if [ -n "$ver" ]; then
      echo "$ver"
      return 0
    fi
  fi

  echo "unknown"
}

BUNDLE=false
BUNDLE_ZIP="${BUNDLE_ZIP:-}"
POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    -b|--bundle)
      BUNDLE=true
      if [[ $# -gt 1 && "$2" == *.zip ]]; then
        BUNDLE_ZIP="$2"
        shift 2
      else
        shift
      fi
      ;;
    --bundle=*)
      BUNDLE=true
      BUNDLE_ZIP="${1#*=}"
      shift
      ;;
    -o|--output-zip)
      BUNDLE=true
      if [[ $# -gt 1 ]]; then
        BUNDLE_ZIP="$2"
        shift 2
      else
        shift
      fi
      ;;
    --output-zip=*)
      BUNDLE=true
      BUNDLE_ZIP="${1#*=}"
      shift
      ;;
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

if [ ${#POSITIONAL[@]} -gt 0 ]; then
  ENVIRONMENTS=("${POSITIONAL[@]}")
else
  ENVIRONMENTS=("cyd_28c" "cyd_28c_inv")
fi

FIRMWARE_VERSION="$(get_firmware_version)"

if [ -n "${BUNDLE_ZIP}" ]; then
  BUNDLE=true
fi

if [ "${BUNDLE}" = true ]; then
  if [ -z "${BUNDLE_ZIP}" ]; then
    if [ -n "${FIRMWARE_VERSION}" ] && [ "${FIRMWARE_VERSION}" != "unknown" ]; then
      BUNDLE_ZIP="${ENVIRONMENTS[0]}_bundle_${FIRMWARE_VERSION}.zip"
    else
      BUNDLE_ZIP="${ENVIRONMENTS[0]}_bundle.zip"
    fi
  fi
  if [[ "${BUNDLE_ZIP}" != *.zip ]]; then
    BUNDLE_ZIP="${BUNDLE_ZIP}.zip"
  fi
  if [[ "${BUNDLE_ZIP}" != */* ]]; then
    BUNDLE_ZIP="${OUTPUT_DIR}/${BUNDLE_ZIP}"
  fi
fi

echo "=========================================="
echo "Packaging binaries for environments: ${ENVIRONMENTS[*]}"
echo "Firmware version: ${FIRMWARE_VERSION}"
echo "Destination directory: ${OUTPUT_DIR}"
if [ "${BUNDLE}" = true ]; then
  echo "Bundle archive: ${BUNDLE_ZIP}"
fi
echo "=========================================="

ENV_VER="${FIRMWARE_VERSION}"

for ENV in "${ENVIRONMENTS[@]}"; do
  echo ""
  echo "--> [1/2] Building environment: ${ENV}..."
  pio run -e "${ENV}"

  BUILD_DIR=".pio/build/${ENV}"
  
  if [ ! -f "${BUILD_DIR}/firmware.bin" ] || [ ! -f "${BUILD_DIR}/partitions.bin" ] || [ ! -f "${BUILD_DIR}/bootloader.bin" ]; then
    echo "Error: Required binary files missing in ${BUILD_DIR}" >&2
    exit 1
  fi

  # Refresh version after pio run to capture any newly generated version.h
  ENV_VER="$(get_firmware_version)"

  if [ "${BUNDLE}" != true ]; then
    if [ -n "${ENV_VER}" ] && [ "${ENV_VER}" != "unknown" ]; then
      ZIP_FILE="${OUTPUT_DIR}/${ENV}_${ENV_VER}.zip"
    else
      ZIP_FILE="${OUTPUT_DIR}/${ENV}.zip"
    fi
    echo "--> [2/2] Creating ZIP archive: ${ZIP_FILE}..."
    
    # Remove existing zip if present
    rm -f "${ZIP_FILE}"

    # Package binaries using python3 zipfile to avoid external zip utility dependency
    python3 -c "
import zipfile
files = [
    ('${BUILD_DIR}/bootloader.bin', 'bootloader.bin'),
    ('${BUILD_DIR}/partitions.bin', 'partitions.bin'),
    ('${BUILD_DIR}/firmware.bin', 'firmware.bin'),
]
with zipfile.ZipFile('${ZIP_FILE}', 'w', compression=zipfile.ZIP_DEFLATED) as zf:
    for src, arc in files:
        zf.write(src, arc)
"

    echo "--> Successfully packaged: ${ZIP_FILE}"
    python3 -c "
import zipfile
with zipfile.ZipFile('${ZIP_FILE}', 'r') as zf:
    zf.printdir()
"
  fi
done

if [ "${BUNDLE}" = true ]; then
  echo ""
  echo "--> [2/2] Creating bundle ZIP archive: ${BUNDLE_ZIP}..."
  rm -f "${BUNDLE_ZIP}"

  python3 -c "
import zipfile

environments = '''${ENVIRONMENTS[*]}'''.split()
bundle_zip = '''${BUNDLE_ZIP}'''
firmware_ver = '''${ENV_VER:-$FIRMWARE_VERSION}'''

files = []
for env in environments:
    build_dir = f'.pio/build/{env}'
    files.append((f'{build_dir}/bootloader.bin', f'{env}/bootloader.bin'))
    files.append((f'{build_dir}/partitions.bin', f'{env}/partitions.bin'))
    files.append((f'{build_dir}/firmware.bin', f'{env}/firmware.bin'))

readme_content = f'''CYD Weather Station Firmware Package
====================================
Version: {firmware_ver}
Environments included:
{', '.join(environments)}

Flashing offsets for each environment:
- bootloader.bin -> 0x1000
- partitions.bin -> 0x8000
- firmware.bin   -> 0x10000
'''

with zipfile.ZipFile(bundle_zip, 'w', compression=zipfile.ZIP_DEFLATED) as zf:
    zf.writestr('README.txt', readme_content)
    for src, arc in files:
        zf.write(src, arc)
"
  echo "--> Successfully created bundle: ${BUNDLE_ZIP}"
  python3 -c "
import zipfile
with zipfile.ZipFile('${BUNDLE_ZIP}', 'r') as zf:
    zf.printdir()
"
fi

echo ""
echo "=========================================="
echo "All archives successfully created in ${OUTPUT_DIR}/"
ls -lh "${OUTPUT_DIR}"/*.zip
echo "=========================================="
