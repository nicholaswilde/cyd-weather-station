---
name: release-summary
description: Generates a clean, professional GitHub release summary based on git logs using scripts/release_summary.py.
---
# /release-summary [range]

Generates a clean, professional GitHub release summary based on git logs.

## Description
This skill uses `scripts/release_summary.py` to retrieve git commit logs, format them into a structured markdown summary (Features, Bug Fixes, etc.), and updates the existing GitHub draft release.

## Protocol

1. Execute the Python script. If no range is provided, it automatically detects the latest two tags.
   ```bash
   ./scripts/release_summary.py [range]
   ```
2. The script will parse the commits, generate the markdown, and automatically run `gh release edit` to update the draft release in-place.
