---
name: release
description: Automates the versioning, tagging, and deployment process for the project using scripts/release.sh.
---
# /release

Automates the versioning, tagging, and deployment process for the project.

## Description
This skill handles the release process by executing the `scripts/release.sh` utility. The script will automatically determine the next patch version, run static analysis and unit tests (`task check`, `task test`), bump `version.txt`, commit, tag, and push atomically to the remote repository.

## Protocol

1. Simply execute the script in the terminal:
   ```bash
   ./scripts/release.sh
   ```
2. If it succeeds, the new version will be tagged and pushed to trigger the GitHub Actions release workflow.
