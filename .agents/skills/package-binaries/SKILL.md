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

### Outputs

The resulting zip files will be placed in the `dist/` folder (or `$OUTPUT_DIR` if overridden):
- `dist/cyd_28c.zip`
- `dist/cyd_28c_inv.zip`

Each zip package contains:
- `bootloader.bin`
- `partitions.bin`
- `firmware.bin`

## Agent Guidelines

- Run this script using `run_command` whenever asked to create, prepare, or package test/release binary zip files for external testers or releases.
