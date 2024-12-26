#!/bin/bash

# Get the root directory of the git repository
GIT_DIR=$(git rev-parse --git-dir)

# Define the source and target for the pre-commit hook
SOURCE_HOOK="scripts/pre-commit"
TARGET_HOOK="$GIT_DIR/hooks/pre-commit"

# Copy the pre-commit hook to the .git/hooks directory
echo "Installing pre-commit hook..."

if [ -f "$SOURCE_HOOK" ]; then
    cp "$SOURCE_HOOK" "$TARGET_HOOK"
    chmod +x "$TARGET_HOOK"
    echo "Pre-commit hook installed successfully."
else
    echo "Pre-commit hook script was not found at $SOURCE_HOOK. Installation failed."
    exit 1
fi
