---
name: issue-manager
description: Automates creating GitHub issues and starting implementation branches using scripts/issue_manager.sh.
---
# /issue-manager

Automates the creation and branch initialization of GitHub issues to save tokens and standardize naming conventions.

## Description
This skill handles the issue workflow. Instead of manually running `gh issue create`, fetching issue IDs, determining branch names (`feat/issue-X` or `fix/issue-X`), and creating branches, agents should simply use `scripts/issue_manager.sh`.

## Protocol

### 1. Creating a New Issue
When asked to create an issue for a bug or feature, run:
```bash
./scripts/issue_manager.sh create <feat|bug> "<title>"
```
*This will automatically prefix the title, assign the `enhancement` or `bug` label, and output the new Issue URL and Number.*

### 2. Starting Implementation
When asked to implement an existing issue (or after creating a new one), run:
```bash
./scripts/issue_manager.sh start <issue_number>
```
*This script will fetch the latest `main`, read the issue's labels to determine if it is a bug or feature, create the properly named branch (e.g., `feat/issue-42` or `fix/issue-42`), check it out, and print the issue details to your context.*
