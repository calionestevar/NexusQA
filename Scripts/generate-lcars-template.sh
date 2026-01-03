#!/bin/bash
#
# Generate-LCARSTemplate.sh
# Extracts HTML template from LCARSTemplate.cpp and writes to LCARSDashboard.html
#
# Single Source of Truth Pattern:
# - LCARSTemplate.cpp contains the authoritative template
# - This script extracts it to LCARSDashboard.html for dev builds
# - Shipped builds use the embedded template from the compiled C++ file
#
# Cross-platform compatible (macOS, Linux)
#

set -e

PROJECT_ROOT="${1:-.}"
CPP_SOURCE_PATH="$PROJECT_ROOT/Source/Nexus/LCARSBridge/Private/LCARSTemplate.cpp"
HTML_OUTPUT_PATH="$PROJECT_ROOT/Source/Nexus/LCARSBridge/Private/LCARSTemplate.html"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "   HTML Template Generator — Observer Network Dashboard"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Validate source file exists
if [ ! -f "$CPP_SOURCE_PATH" ]; then
    echo "[✗] Source file not found: $CPP_SOURCE_PATH"
    exit 1
fi

echo "[+] Source: $CPP_SOURCE_PATH"
echo "[+] Output: $HTML_OUTPUT_PATH"
echo ""

# Extract HTML from raw string literal R"(...)"
# sed: 
#   - Find the line with R"(
#   - Extract everything from R"( to )"
#   - Remove the R"( prefix and )" suffix

if sed -n '/R"(/,/)"/{s/^.*R"(//;s/)".*$//p;}' "$CPP_SOURCE_PATH" > "$HTML_OUTPUT_PATH"; then
    FILE_SIZE=$(stat -f%z "$HTML_OUTPUT_PATH" 2>/dev/null || stat -c%s "$HTML_OUTPUT_PATH")
    FILE_SIZE_KB=$((FILE_SIZE / 1024))
    
    echo "[✓] Successfully extracted HTML template"
    echo "[✓] File size: ${FILE_SIZE_KB} KB"
    echo "[✓] Location: $HTML_OUTPUT_PATH"
    echo ""
    echo "Generated from: LCARSTemplate.cpp"
    echo "This is the development/packaged builds version."
    echo "Shipped builds use the embedded C++ template."
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
else
    echo "[✗] Failed to extract HTML template from C++ source"
    exit 1
fi
