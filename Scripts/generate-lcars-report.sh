#!/bin/bash
#
# generate-lcars-report.sh
# Generate LCARS Enhanced Report with metrics dashboard
# Cross-platform compatible (macOS, Linux)
# Usage: ./generate-lcars-report.sh [--open]
#
# Template Architecture: Single Source of Truth
# - Template loaded from LCARSTemplate.html (extracted from LCARSTemplate.cpp)
# - To update template, edit LCARSTemplate.cpp and run generate-lcars-template.sh
#

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TEMPLATE_PATH="$PROJECT_ROOT/Source/Nexus/LCARSBridge/Private/LCARSTemplate.html"

REPORT_DIR="Saved/NexusReports"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
REPORT_PATH="$REPORT_DIR/LCARS_Report_$TIMESTAMP.html"
OPEN_BROWSER=false

# Validate template exists
if [ ! -f "$TEMPLATE_PATH" ]; then
    echo "[ERROR] Template file not found: $TEMPLATE_PATH"
    echo ""
    echo "To generate the template, run:"
    echo "   ./Scripts/generate-lcars-template.sh"
    exit 1
fi

# Parse arguments
if [[ "$1" == "--open" ]]; then
    OPEN_BROWSER=true
fi

# Create directory if it doesn't exist
mkdir -p "$REPORT_DIR"

echo "Generating enhanced LCARS report..."

# Calculate sample metrics
TOTAL_TESTS=17
PASSED_TESTS=14
FAILED_TESTS=3
INTEGRITY_PERCENT=$((100 * PASSED_TESTS / TOTAL_TESTS))
AVG_DURATION=542
CRITICAL_TESTS=5
REGRESSION_COUNT=2
PERF_STATUS="Good"
REGRESSION_STATUS="Investigate"
STARDATE=$(date '+%Y-%m-%d %H:%M:%S')

# Determine integrity class
if (( INTEGRITY_PERCENT < 70 )); then
    INTEGRITY_CLASS="critical"
elif (( INTEGRITY_PERCENT < 85 )); then
    INTEGRITY_CLASS="warning"
else
    INTEGRITY_CLASS=""
fi

# Generate tag distribution cards HTML
TAG_DISTRIBUTION_CARDS=$(cat <<'TAGS'
                <div class="tag-card">
                    <div class="count">2</div>
                    <div class="label">Networking</div>
                </div>
                <div class="tag-card">
                    <div class="count">2</div>
                    <div class="label">Performance</div>
                </div>
                <div class="tag-card">
                    <div class="count">3</div>
                    <div class="label">Gameplay</div>
                </div>
                <div class="tag-card">
                    <div class="count">2</div>
                    <div class="label">Compliance</div>
                </div>
                <div class="tag-card">
                    <div class="count">2</div>
                    <div class="label">Integration</div>
                </div>
                <div class="tag-card">
                    <div class="count">2</div>
                    <div class="label">Stress</div>
                </div>
                <div class="tag-card">
                    <div class="count">1</div>
                    <div class="label">Editor</div>
                </div>
                <div class="tag-card">
                    <div class="count">2</div>
                    <div class="label">Rendering</div>
                </div>
TAGS
)

# Generate grouped test sections
GROUPED_TEST_SECTIONS=$(cat <<'GROUPS'
        <div class="tag-section">
            <div class="tag-section-header" onclick="toggleSection(this)">
                <span>Networking Tests</span>
                <span class="toggle-icon">▼</span>
            </div>
            <div class="tag-section-stats">2 tests - 100% passed</div>
            <div class="tag-section-content">
                <table class="tag-test-table">
                    <tr><td class="test-passed">Test_NetworkRequest_Basic - PASSED</td></tr>
                    <tr><td class="test-passed">Test_NetworkRequest_Timeout - PASSED</td></tr>
                </table>
            </div>
        </div>
GROUPS
)

# Generate table rows
ALL_TESTS_TABLE_ROWS=$(cat <<'ROWS'
                <tr><td class='test-name'>Test_NetworkRequest_Basic</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_NetworkRequest_Timeout</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_NetworkRequest_Retry</td><td class='test-failed'>FAILED</td></tr>
                <tr><td class='test-name'>Test_PerformanceBaseline</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_PerformanceStress</td><td class='test-failed'>FAILED</td></tr>
                <tr><td class='test-name'>Test_GameplayMovement</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_GameplayCollision</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_GameplayAI</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_ComplianceGDPR</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_ComplianceCOPPA</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_IntegrationDatabase</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_IntegrationCache</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_StressLoad</td><td class='test-failed'>FAILED</td></tr>
                <tr><td class='test-name'>Test_StressMemory</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_EditorPlugin</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_RenderingMesh</td><td class='test-passed'>PASSED</td></tr>
                <tr><td class='test-name'>Test_RenderingShader</td><td class='test-passed'>PASSED</td></tr>
ROWS
)

# Load template and perform substitutions
TEMPLATE_CONTENT=$(cat "$TEMPLATE_PATH")

# Replace all placeholders
HTML=$(echo "$TEMPLATE_CONTENT" | \
    sed "s|{STARDATE}|$STARDATE|g" | \
    sed "s|{INTEGRITY_PERCENT}|$INTEGRITY_PERCENT|g" | \
    sed "s|{INTEGRITY_CLASS}|$INTEGRITY_CLASS|g" | \
    sed "s|{PASSED_TESTS}|$PASSED_TESTS|g" | \
    sed "s|{TOTAL_TESTS}|$TOTAL_TESTS|g" | \
    sed "s|{CRITICAL_TESTS}|$CRITICAL_TESTS|g" | \
    sed "s|{AVG_DURATION}|$AVG_DURATION|g" | \
    sed "s|{PERF_STATUS}|$PERF_STATUS|g" | \
    sed "s|{REGRESSION_COUNT}|$REGRESSION_COUNT|g" | \
    sed "s|{REGRESSION_STATUS}|$REGRESSION_STATUS|g")

# For complex replacements, use a temporary marker approach
HTML=$(echo "$HTML" | sed "/<!-- TEMPLATE_TAG_CARDS -->/c\\
$TAG_DISTRIBUTION_CARDS")

HTML=$(echo "$HTML" | sed "/<!-- TEMPLATE_GROUPED_SECTIONS -->/c\\
$GROUPED_TEST_SECTIONS")

HTML=$(echo "$HTML" | sed "/<!-- TEMPLATE_TEST_ROWS -->/c\\
$ALL_TESTS_TABLE_ROWS")

# Write to file
echo "$HTML" > "$REPORT_PATH"

echo "[SUCCESS] Report generated successfully!"
echo "Location: $REPORT_PATH"
echo ""

if [ "$OPEN_BROWSER" = true ]; then
    # Try to open in default browser (platform-aware)
    if command -v open &> /dev/null; then
        # macOS
        open "$REPORT_PATH"
    elif command -v xdg-open &> /dev/null; then
        # Linux
        xdg-open "$REPORT_PATH"
    else
        echo "Could not determine how to open browser on this system"
        echo "Open manually: $REPORT_PATH"
    fi
    echo "Opening in browser..."
else
    echo "To open in browser, run:"
    echo "   open '$REPORT_PATH'  # macOS"
    echo "   xdg-open '$REPORT_PATH'  # Linux"
fi

echo ""
echo "Enhanced LCARS dashboard features:"
echo "  - System Integrity card with status bar"
echo "  - 4-column metrics grid (Performance, Regression, Memory, Frames)"
echo "  - Test distribution by category"
echo "  - Collapsible test sections with auto-expand for failures"
echo "  - Complete flat test listing for reference"
