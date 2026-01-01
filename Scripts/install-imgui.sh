#!/bin/bash
#
# install-imgui.sh
# Download and install an ImGui plugin into the project-local Plugins/ImGui folder.
# Cross-platform compatible (macOS, Linux)
#
# Usage:
#   ./Scripts/install-imgui.sh [URL] [ENGINE_PATH]
#
# If no URL is supplied, the script will prompt you to paste a zip URL (GitHub release or raw zip).
# The script will extract the archive into Plugins/ImGui (overwriting the stub) and then
# generate project files using your configured UE_ENGINE_PATH or the ENGINE_PATH argument.
#

set -e

URL="$1"
ENGINE_PATH="$2"

if [ -z "$URL" ]; then
    echo "Enter a download URL to an ImGui plugin zip (GitHub release, etc.):"
    read -r URL
fi

if [ -z "$URL" ]; then
    echo "No URL provided. Aborting."
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
PLUGINS_DIR="$REPO_ROOT/Plugins"
TARGET_DIR="$PLUGINS_DIR/ImGui"
TMP_FILE=$(mktemp /tmp/imgui.XXXXXX.zip)

echo "Downloading ImGui plugin from: $URL"

# Use curl to download (more portable than wget)
if ! curl -L -o "$TMP_FILE" "$URL" 2>/dev/null; then
    echo "Failed to download from URL"
    rm -f "$TMP_FILE"
    exit 1
fi

if [ -d "$TARGET_DIR" ]; then
    echo "Removing existing $TARGET_DIR"
    rm -rf "$TARGET_DIR"
fi

echo "Extracting plugin to $PLUGINS_DIR"
unzip -q "$TMP_FILE" -d "$PLUGINS_DIR"

# Try to find a top-level folder that contains an ImGui.uplugin
found=$(find "$PLUGINS_DIR" -maxdepth 2 -name "ImGui.uplugin" 2>/dev/null | head -n 1)

if [ -z "$found" ]; then
    echo "Warning: Could not find ImGui.uplugin in the extracted archive."
    echo "Please move the plugin folder into Plugins/ImGui manually."
else
    plugin_dir=$(dirname "$found")
    if [ "$plugin_dir" != "$TARGET_DIR" ]; then
        echo "Moving plugin folder to $TARGET_DIR"
        mv "$plugin_dir" "$TARGET_DIR"
    fi
    echo "ImGui plugin installed to $TARGET_DIR"
fi

rm -f "$TMP_FILE"

if [ -n "$ENGINE_PATH" ] && [ -f "$ENGINE_PATH/Build/BatchFiles/GenerateProjectFiles.sh" ]; then
    echo "Generating project files using engine at $ENGINE_PATH"
    "$ENGINE_PATH/Build/BatchFiles/GenerateProjectFiles.sh" -projectfiles -project="$REPO_ROOT/NexusDemo.uproject" -game -engine
elif [ -n "$UE_ENGINE_PATH" ] && [ -f "$UE_ENGINE_PATH/Build/BatchFiles/GenerateProjectFiles.sh" ]; then
    echo "Generating project files using engine at $UE_ENGINE_PATH"
    "$UE_ENGINE_PATH/Build/BatchFiles/GenerateProjectFiles.sh" -projectfiles -project="$REPO_ROOT/NexusDemo.uproject" -game -engine
else
    echo "Plugin installed. Run GenerateProjectFiles.sh with your engine to pick it up, e.g."
    echo "\$ENGINE_PATH/Build/BatchFiles/GenerateProjectFiles.sh -projectfiles -project=\"$REPO_ROOT/NexusDemo.uproject\" -game -engine"
fi

echo "Done."
