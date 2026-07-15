# /test-fix

Runs the full validation suite (static analysis and unit tests) and autonomously fixes failures.

## Description
This skill ensures the `cyd-weather-station` codebase conforms to static code analysis, linting, quality, and testing standards. It actively detects violations and applies automatic or surgical corrections.

## Protocol

1. **Execute CI Checks:**
   - Run the validation tasks:
     - Linting / Static Code Analysis: Run `task check` (equivalent to `pio check`)
     - Unit Tests: Run `task test` (equivalent to `pio test -e native`)

2. **Analyze Failures:**
   - If any check fails, capture the output and identify the category:
     - **Static Analysis / Linting**: C++ compilation warnings, code smell, or quality warnings found by cppcheck.
     - **Unit Tests**: Test assertion failures, mock object mismatches, or setup issues in `tests/`.
     - **CRITICAL:** Do NOT modify `Taskfile.yml` to bypass or disable any checks.

3. **Apply Corrections:**
   - **Lints/Cppcheck**: Surgical corrections in the code. Address the lint recommendations directly, or add appropriate suppressions in `platformio.ini` under `check_flags` if they are false positives.
   - **Tests**: Debug code logic, update test assertions, or correct mock objects in `tests/mocks/` as needed.

4. **Verify and Re-Test:**
   - Re-run the tests via `task test` and checks via `task check` to ensure all checks pass cleanly after modifications.

5. **Report:**
   - Provide a concise summary of the checks executed and any fixes applied.
