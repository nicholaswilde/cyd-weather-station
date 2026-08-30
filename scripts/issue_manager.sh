#!/usr/bin/env bash
set -e

COMMAND=$1

if [ "$COMMAND" = "create" ]; then
    TYPE=$2 # 'feat' or 'bug'
    TITLE=$3
    if [ -z "$TITLE" ] || [ -z "$TYPE" ]; then
        echo "Usage: $0 create <feat|bug> <\"issue title\">"
        exit 1
    fi
    
    LABEL="enhancement"
    if [ "$TYPE" = "bug" ]; then
        LABEL="bug"
    fi
    
    ISSUE_URL=$(gh issue create --title "[$TYPE]: $TITLE" --body "Created via issue-manager automation." --label "$LABEL")
    ISSUE_NUM=$(echo "$ISSUE_URL" | grep -oE '[0-9]+$')
    
    echo "Issue created: $ISSUE_URL"
    echo "To start working on it, run: $0 start $ISSUE_NUM"

elif [ "$COMMAND" = "start" ]; then
    ISSUE_NUM=$2
    if [ -z "$ISSUE_NUM" ]; then
        echo "Usage: $0 start <issue_number>"
        exit 1
    fi
    
    LABELS=$(gh issue view "$ISSUE_NUM" --json labels --jq '.labels[].name' || echo "")
    
    PREFIX="feat"
    if echo "$LABELS" | grep -iq "bug"; then
        PREFIX="fix"
    fi
    
    BRANCH_NAME="$PREFIX/issue-$ISSUE_NUM"
    
    # Sync main first
    git checkout main
    git pull --rebase origin main
    
    echo "Creating and checking out branch: $BRANCH_NAME"
    git checkout -b "$BRANCH_NAME"
    
    echo "Ready to implement issue #$ISSUE_NUM on branch $BRANCH_NAME"
    echo "Issue Details:"
    gh issue view "$ISSUE_NUM" | cat

else
    echo "GitHub Issue Manager"
    echo "Usage:"
    echo "  $0 create <feat|bug> <\"title\">   - Create a new issue with proper labels"
    echo "  $0 start <issue_number>          - Checkout a new branch formatted for the issue"
    exit 1
fi
