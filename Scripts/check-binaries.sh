#!/bin/bash
#
# check-binaries.sh
# Simple helper to sanity-check compiled plugin binaries and project receipts.
# Cross-platform compatible (macOS, Linux)
#
# Usage:
#   ./Scripts/check-binaries.sh [-v|--verbose]
#

set -e

VERBOSE=false
if [ "$1" == "-v" ] || [ "$1" == "--verbose" ]; then
    VERBOSE=true
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Project root: $PROJECT_ROOT"
echo ""

# Determine architecture suffix (macOS uses different naming)
if [[ "$OSTYPE" == "darwin"* ]]; then
    ARCH_SUFFIX="Mac"
else
    ARCH_SUFFIX="Linux"
fi

# Check for project-level Binaries receipts
PROJECT_BINARIES_DIR="$PROJECT_ROOT/Binaries/$ARCH_SUFFIX"
if [ -d "$PROJECT_BINARIES_DIR" ]; then
    TARGET_FILES=$(find "$PROJECT_BINARIES_DIR" -name "*.target" 2>/dev/null | wc -l)
    if [ "$TARGET_FILES" -eq 0 ]; then
        echo "⚠ Warning: No .target receipts found in $PROJECT_BINARIES_DIR"
    else
        echo "✓ Found .target receipts:"
        find "$PROJECT_BINARIES_DIR" -name "*.target" -exec basename {} \; | sed 's/^/ - /'
    fi
else
    echo "⚠ No Binaries/$ARCH_SUFFIX folder at $PROJECT_BINARIES_DIR"
fi

echo ""

# Inspect project Plugins
PLUGINS_DIR="$PROJECT_ROOT/Plugins"
if [ ! -d "$PLUGINS_DIR" ]; then
    echo "⚠ No Plugins folder at $PLUGINS_DIR"
else
    PLUGIN_COUNT=$(find "$PLUGINS_DIR" -maxdepth 1 -type d ! -name "Plugins" 2>/dev/null | wc -l)
    
    if [ "$PLUGIN_COUNT" -eq 0 ]; then
        echo "No plugin directories found."
    else
        for PLUGIN_PATH in "$PLUGINS_DIR"/*; do
            if [ ! -d "$PLUGIN_PATH" ]; then
                continue
            fi
            
            PLUGIN_NAME=$(basename "$PLUGIN_PATH")
            echo "Plugin: $PLUGIN_NAME"
            
            UPLUGIN=$(find "$PLUGIN_PATH" -name "*.uplugin" 2>/dev/null | head -n 1)
            if [ -z "$UPLUGIN" ]; then
                echo " ⚠ No .uplugin file found in plugin folder."
                continue
            fi
            
            echo " - Descriptor: $UPLUGIN"
            
            # Try to parse JSON and extract module names
            if command -v jq &> /dev/null; then
                MODULES=$(jq -r '.modules[]?.name' "$UPLUGIN" 2>/dev/null || echo "")
            else
                # Fallback: grep for module names (basic)
                MODULES=$(grep -o '"name": "[^"]*"' "$UPLUGIN" | grep -v description | cut -d'"' -f4 | sort -u)
            fi
            
            # Check for binaries
            PLUGIN_BIN_DIR="$PLUGIN_PATH/Binaries/$ARCH_SUFFIX"
            if [ -d "$PLUGIN_BIN_DIR" ]; then
                DLLS=$(find "$PLUGIN_BIN_DIR" -name "*.dylib" -o -name "*.so" 2>/dev/null | wc -l)
                if [ "$DLLS" -eq 0 ]; then
                    echo " ⚠ No $ARCH_SUFFIX binaries found under $PLUGIN_BIN_DIR"
                else
                    echo " ✓ Binaries found:"
                    find "$PLUGIN_BIN_DIR" -type f \( -name "*.dylib" -o -name "*.so" \) -exec basename {} \; | sed 's/^/    - /'
                fi
            else
                echo " ⚠ No Binaries/$ARCH_SUFFIX folder"
            fi
            
            # Check if modules are present
            if [ -z "$MODULES" ]; then
                echo " ⚠ No modules declared in .uplugin"
            else
                while IFS= read -r MODULE; do
                    [ -z "$MODULE" ] && continue
                    
                    # Check if binary exists in plugin folder
                    if [ -d "$PLUGIN_BIN_DIR" ] && find "$PLUGIN_BIN_DIR" -name "*$MODULE*" \( -name "*.dylib" -o -name "*.so" \) &>/dev/null; then
                        echo " ✓ Module $MODULE: binary present"
                    else
                        # Check project-level fallback
                        if find "$PROJECT_BINARIES_DIR" -name "*$MODULE*" \( -name "*.dylib" -o -name "*.so" \) &>/dev/null 2>/dev/null; then
                            echo " ✓ Module $MODULE: binary present in project Binaries"
                        else
                            echo " ✗ Module $MODULE: MISSING binary"
                        fi
                    fi
                done <<< "$MODULES"
            fi
            
            echo ""
        done
    fi
fi

echo "Check complete."
