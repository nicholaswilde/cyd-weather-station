---
name: package-binaries
description: Skill to build and package firmware binary ZIP files (partitions.bin, firmware.bin, bootloader.bin) for CYD environments (such as cyd_28c and cyd_28c_inv).
---

# Package Binaries Skill

This skill builds and bundles PlatformIO binary outputs into individual ZIP files containing `bootloader.bin`, `partitions.bin`, and `firmware.bin` for manual distribution and hardware testing.

## Usage

To generate zip archives for the default `cyd_28c` and `cyd_28c_inv` boards:

```bash
bash .agents/skills/package-binaries/package_binaries.sh
```

To generate zip archives for specific environments:

```bash
bash .agents/skills/package-binaries/package_binaries.sh cyd_28c cyd_28c_inv
# or other targets:
bash .agents/skills/package-binaries/package_binaries.sh cyd_28r cyd_35c
```

To bundle multiple environments into a single zip file:

```bash
bash .agents/skills/package-binaries/package_binaries.sh --bundle cyd_28_nm_bundle.zip cyd_28_nm cyd_28_nm_inv
```

### Outputs

The resulting zip files will be placed in the `dist/` folder (or `$OUTPUT_DIR` if overridden), tagged with the firmware version and commit info (matching the version shown below the app title in the GUI):
- `dist/cyd_28c_<version>-<commit>.zip` (e.g. `dist/cyd_28c_v0.1.38-4-g096e608.zip`)
- `dist/cyd_28c_inv_<version>-<commit>.zip` (e.g. `dist/cyd_28c_inv_v0.1.38-4-g096e608.zip`)
- Default bundle archive: `dist/<first_env>_bundle_<version>-<commit>.zip`

Each zip package contains:
- `bootloader.bin`
- `partitions.bin`
- `firmware.bin`

## Agent Guidelines

- Run this script using `run_command` whenever asked to create, prepare, or package test/release binary zip files for external testers or releases.
