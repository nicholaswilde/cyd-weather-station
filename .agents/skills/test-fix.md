# /test-fix

Runs the full CI test suite (format, lint, tests, and coverage) and autonomously fixes failures.

## Description
This skill ensures the `sysmqttd` system monitoring daemon codebase conforms to formatting, linting, quality, and testing standards. It actively detects violations and applies automatic or surgical corrections.

## Protocol

1. **Execute CI Checks:**
   - Run the validation tasks in sequence:
     - Formatting: Run `cargo fmt --all -- --check` or check `task format` compliance.
     - Linting & Clippy: Run `task lint`
     - Unit & Integration Tests: Run `task test`
     - Code Coverage: Run `task coverage` to ensure total line coverage exceeds 90%.

2. **Analyze Failures:**
   - If any check fails, capture the output and identify the category:
     - **Formatting**: Code formatting style mismatch.
     - **Linting/Clippy**: Code quality warnings or compiler-recommended lints.
     - **Unit/Integration Tests**: Test assertion failures, unexpected panics, or mock/event loop broker configuration issues.
     - **Code Coverage**: Fails to meet the 90%+ line coverage gate.

3. **Apply Corrections:**
   - **Formatting**: Run `task format` to automatically fix formatting.
   - **Lints/Clippy**: Surgical corrections in the code. Address the lint recommendations directly, or add `#[allow(...)]` attributes if a lint is a false positive.
   - **Tests**: Debug code logic, update test assertions, or correct mock objects as needed.
   - **Coverage**: Add targeted unit/integration tests to cover untested files, branches, or fallback blocks (e.g. mocking `/sys/` class files or using custom mock roots).
   - **CRITICAL:** Do NOT modify `Taskfile.yml` to bypass or disable any checks.

4. **Verify and Re-Test:**
   - Re-run the tests via `task test` and `task coverage` to ensure all checks pass cleanly after your modifications.

5. **Report:**
   - Provide a concise summary of the checks executed and any fixes applied.
