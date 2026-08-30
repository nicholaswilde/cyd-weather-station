---
name: issue-manager
description: Automates creating GitHub issues and starting implementation branches using scripts/issue_manager.sh.
---
# /issue-manager

Automates the creation, branch initialization, and resolution of GitHub issues to save tokens and standardize conventions.

## Description
This skill handles the issue workflow. Instead of manually running git and gh CLI commands for every step of an issue's lifecycle, agents should simply use `scripts/issue_manager.sh`.

## Protocol

### 1. Creating a New Issue
When asked to create an issue for a bug or feature, run:
```bash
./scripts/issue_manager.sh create <feat|bug> "<title>"
```

### 2. Starting Implementation
When asked to implement an existing issue (or after creating a new one), run:
```bash
./scripts/issue_manager.sh start <issue_number>
```

### 3. Finishing & Merging
When the implementation is complete and tested, merge it by running (while on the issue branch):
```bash
./scripts/issue_manager.sh finish
```
*This will automatically checkout main, merge the issue branch, push main, delete the local branch, and close the issue on GitHub.*

### 4. Submitting a Pull Request
If the workflow requires a PR instead of a direct merge, run (while on the issue branch):
```bash
./scripts/issue_manager.sh submit
```
*This pushes the issue branch and uses `gh pr create` to generate a PR linked to the issue.*
