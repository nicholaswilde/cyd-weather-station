---
name: agent-screen-reader
description: Skill to read the current device screen to get visual context about issues or improvements. Changes the screen to the specified tab and takes a screenshot for the agent to view.
---

# Agent Screen Reader Skill

This skill allows the agent to take a screenshot of a specific tab on the CYD Weather Station device and read it using the `view_file` tool. 

## Prerequisites
- The device must be on the network.
- `CYD_DEVICE_IP` must be defined in the `.env` file at the root of the project.

## How to capture a screen for analysis

1. Run the `agent-read-screen.sh` script to capture the screen you want to view. You should use `ctx_execute` with `language: shell` if `run_command` is blocked:
   ```shell
   ./scripts/agent-read-screen.sh <tab>
   ```
   `<tab>` can be `current`, `forecast`, `hourly`, or `settings`.

2. The script will output the path to the saved PNG screenshot, for example: `screenshots/agent_settings.png`.

3. Use your `view_file` tool to read the generated PNG file (or BMP if PNG conversion failed). Your `view_file` tool supports reading image files, which will allow you to see the UI exactly as it appears on the device so you can troubleshoot or review it with the user.
