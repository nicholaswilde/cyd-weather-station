# /build-fix

Builds the project firmware, fixes compilation issues, and runs the test-fix skill upon success.

## Description
This skill compiles the `cyd-weather-station` firmware for the ESP32 Cheap Yellow Device, handles any compilation issues, and ensures code quality by running subsequent validation tests.

## Protocol

1. **Execute Build:**
   - Run the PlatformIO build command: `task build` (equivalent to `pio run`).

2. **Analyze and Fix Errors:**
   - If the build fails:
     - Capture and analyze compiler error logs.
     - Identify resolution strategies for PlatformIO/Arduino compiler errors (e.g., missing library dependencies, header path config, or target configuration).
     - **IMPORTANT:** Do NOT modify `Taskfile.yml` to bypass errors. Edit code files, header files, or library dependencies in `platformio.ini` instead.
     - Iterate build executions and surgical fixes until the compilation passes.

3. **Verify Overall Integrity:**
   - Once the build succeeds, run the `/test-fix` skill to ensure overall project integrity (formatting, linting, unit/integration testing).

4. **Completion:**
   - Confirm the successful build and report the results of the subsequent test phase.
