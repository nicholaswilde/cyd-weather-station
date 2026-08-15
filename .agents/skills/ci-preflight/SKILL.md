---
name: ci-preflight
description: Skill to automate local CI pre-flight checks, ensuring firmware compiles and unit tests pass before committing.
---

# CI Pre-Flight Check Skill

This skill provides an automated script to verify code stability before finalizing changes. It builds the firmware for all supported environments and runs host-native unit tests.

## Usage

Before committing code or submitting a pull request, agents or users should run the pre-flight check script from the repository root:

```bash
bash .agents/skills/ci-preflight/preflight.sh
```

### What it does

1. **Builds for `cyd_28r`**: Compiles the project using the PlatformIO `cyd_28r` environment.
2. **Builds for `cyd_35c`**: Compiles the project using the PlatformIO `cyd_35c` environment.
3. **Runs Native Tests**: Executes host-native unit tests using the PlatformIO `native` environment (`pio test -e native`).

If any of these steps fail, the script will exit immediately with a non-zero exit code and display an error, preventing you from pushing broken code.

## Agent Guidelines

- When prompted to run tests or perform a pre-flight check, execute this script using `ctx_execute` or `run_command`.
- If you expect the output to be very long and you only care about failures, you can pipe the script output to `rtk err` or `rtk test` as per `AGENTS.md` guidelines. (e.g. `rtk err bash .agents/skills/ci-preflight/preflight.sh`)
