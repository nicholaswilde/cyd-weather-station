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
    
    ISSUE_URL=$(rtk gh issue create --title "[$TYPE]: $TITLE" --body "Created via issue-manager automation." --label "$LABEL")
    ISSUE_NUM=$(echo "$ISSUE_URL" | grep -oE '[0-9]+$')
    
    echo "Issue created: $ISSUE_URL"
    echo "To start working on it, run: $0 start $ISSUE_NUM"

elif [ "$COMMAND" = "start" ]; then
    ISSUE_NUM=$2
    if [ -z "$ISSUE_NUM" ]; then
        echo "Usage: $0 start <issue_number>"
        exit 1
    fi
    
    LABELS=$(rtk gh issue view "$ISSUE_NUM" --json labels --jq '.labels[].name' || echo "")
    
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
    rtk gh issue view "$ISSUE_NUM" | cat

elif [ "$COMMAND" = "finish" ]; then
    CURRENT_BRANCH=$(git branch --show-current)
    if [[ ! "$CURRENT_BRANCH" =~ (feat|fix)/issue-([0-9]+) ]]; then
        echo "Error: Current branch '$CURRENT_BRANCH' does not match pattern (feat|fix)/issue-<number>."
        exit 1
    fi
    ISSUE_NUM="${BASH_REMATCH[2]}"

    echo "Merging $CURRENT_BRANCH into main..."
    git checkout main
    git pull --rebase origin main
    git merge "$CURRENT_BRANCH"
    
    echo "Pushing main..."
    git push origin main
    
    echo "Cleaning up local branch..."
    git branch -d "$CURRENT_BRANCH"
    
    echo "Closing issue #$ISSUE_NUM..."
    rtk gh issue close "$ISSUE_NUM"
    echo "Issue #$ISSUE_NUM closed successfully."

elif [ "$COMMAND" = "submit" ]; then
    CURRENT_BRANCH=$(git branch --show-current)
    if [[ ! "$CURRENT_BRANCH" =~ (feat|fix)/issue-([0-9]+) ]]; then
        echo "Error: Current branch '$CURRENT_BRANCH' does not match pattern (feat|fix)/issue-<number>."
        exit 1
    fi
    ISSUE_NUM="${BASH_REMATCH[2]}"

    echo "Pushing $CURRENT_BRANCH to origin..."
    git push -u origin "$CURRENT_BRANCH"
    
    echo "Creating Pull Request for issue #$ISSUE_NUM..."
    PR_URL=$(rtk gh pr create --fill --body "Resolves #$ISSUE_NUM" | cat)
    echo "Pull Request created: $PR_URL"

else
    echo "GitHub Issue Manager"
    echo "Usage:"
    echo "  $0 create <feat|bug> <\"title\">   - Create a new issue with proper labels"
    echo "  $0 start <issue_number>          - Checkout a new branch formatted for the issue"
    echo "  $0 finish                        - Merge current branch to main, push, and close issue"
    echo "  $0 submit                        - Push current branch and create a Pull Request"
    exit 1
fi
