#!/usr/bin/env bash
# ==============================================================================
# package_binaries.sh
# --------------------
# Compiles firmware for specified PlatformIO environments and creates ZIP archives
# containing partitions.bin, firmware.bin, and bootloader.bin for testing/flashing.
#
# Usage:
#   bash .agents/skills/package-binaries/package_binaries.sh [ENV1] [ENV2] ...
#
# Defaults to: cyd_28c cyd_28c_inv (if no environments are passed)
# ==============================================================================

set -euo pipefail

# Output directory for release / test zip packages
OUTPUT_DIR="${OUTPUT_DIR:-dist}"
mkdir -p "${OUTPUT_DIR}"

# Target environments (default to cyd_28c and cyd_28c_inv if none provided)
ENVIRONMENTS=("$@")
if [ ${#ENVIRONMENTS[@]} -eq 0 ]; then
  ENVIRONMENTS=("cyd_28c" "cyd_28c_inv")
fi

echo "=========================================="
echo "Packaging binaries for environments: ${ENVIRONMENTS[*]}"
echo "Destination directory: ${OUTPUT_DIR}"
echo "=========================================="

for ENV in "${ENVIRONMENTS[@]}"; do
  echo ""
  echo "--> [1/3] Building environment: ${ENV}..."
  pio run -e "${ENV}"

  BUILD_DIR=".pio/build/${ENV}"
  
  if [ ! -f "${BUILD_DIR}/firmware.bin" ] || [ ! -f "${BUILD_DIR}/partitions.bin" ] || [ ! -f "${BUILD_DIR}/bootloader.bin" ]; then
    echo "Error: Required binary files missing in ${BUILD_DIR}" >&2
    exit 1
  fi

  ZIP_FILE="${OUTPUT_DIR}/${ENV}.zip"
  echo "--> [2/3] Creating ZIP archive: ${ZIP_FILE}..."
  
  # Remove existing zip if present
  rm -f "${ZIP_FILE}"

  # Package binaries using python3 zipfile to avoid external zip utility dependency
  python3 -c "
import zipfile, os
files = [
    ('${BUILD_DIR}/bootloader.bin', 'bootloader.bin'),
    ('${BUILD_DIR}/partitions.bin', 'partitions.bin'),
    ('${BUILD_DIR}/firmware.bin', 'firmware.bin'),
]
with zipfile.ZipFile('${ZIP_FILE}', 'w', compression=zipfile.ZIP_DEFLATED) as zf:
    for src, arc in files:
        zf.write(src, arc)
"

  echo "--> [3/3] Successfully packaged: ${ZIP_FILE}"
  python3 -c "
import zipfile
with zipfile.ZipFile('${ZIP_FILE}', 'r') as zf:
    zf.printdir()
"
done

echo ""
echo "=========================================="
echo "All archives successfully created in ${OUTPUT_DIR}/"
ls -lh "${OUTPUT_DIR}"/*.zip
echo "=========================================="
