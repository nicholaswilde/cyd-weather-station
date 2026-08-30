#!/usr/bin/env bash
set -e

# Extract and calculate next version
LATEST_TAG=$(git tag --sort=-v:refname | head -n 1)
CURRENT_VER=${LATEST_TAG#v}

IFS='.' read -ra VER_PARTS <<< "$CURRENT_VER"
PATCH=${VER_PARTS[2]}
NEW_PATCH=$((PATCH + 1))
NEW_VER="${VER_PARTS[0]}.${VER_PARTS[1]}.$NEW_PATCH"
NEW_TAG="v$NEW_VER"

echo "Current version: $LATEST_TAG"
echo "Target version: $NEW_TAG"

echo "Running pre-release validation (check & test)..."
task check
task test

echo "Validation passed. Bumping version.txt..."
echo "$NEW_TAG" > version.txt
git add version.txt
git commit -m "chore(release): bump version to $NEW_TAG"

echo "Checking git status..."
if [ -n "$(git status --porcelain)" ]; then
    echo "Error: Working directory not clean after bumping version."
    exit 1
fi

echo "Pulling latest changes..."
git pull --rebase origin main

echo "Tagging release..."
git tag -a "$NEW_TAG" -m "Release $NEW_TAG"

echo "Pushing atomically..."
git push --atomic origin main "$NEW_TAG"

echo "Release $NEW_TAG published successfully!"
