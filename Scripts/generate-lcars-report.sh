#!/bin/bash
#
# generate-lcars-report.sh
# Generate LCARS Enhanced Report with metrics dashboard
# Cross-platform compatible (macOS, Linux)
# Usage: ./generate-lcars-report.sh [--open]
#

REPORT_DIR="Saved/NexusReports"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
REPORT_PATH="$REPORT_DIR/LCARS_Report_$TIMESTAMP.html"
OPEN_BROWSER=false

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

# Determine integrity class
if (( INTEGRITY_PERCENT < 70 )); then
    INTEGRITY_CLASS="critical"
elif (( INTEGRITY_PERCENT < 85 )); then
    INTEGRITY_CLASS="warning"
else
    INTEGRITY_CLASS=""
fi

# Generate HTML report
cat > "$REPORT_PATH" << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>LCARS NEXUS FINAL REPORT</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            background: linear-gradient(135deg, #000033 0%, #001a66 100%);
            color: #ffcc00;
            font-family: 'Courier New', monospace;
            padding: 40px 20px;
            line-height: 1.6;
        }
        .lcars-frame {
            max-width: 1400px;
            margin: 0 auto;
            border: 3px solid #ff9900;
            border-radius: 30px;
            padding: 50px;
            background: radial-gradient(ellipse at center, #000066 0%, #000033 100%);
            box-shadow: 0 0 40px rgba(255, 153, 0, 0.5), inset 0 0 20px rgba(255, 153, 0, 0.1);
        }
        .lcars-header {
            text-align: center;
            margin-bottom: 50px;
            border-bottom: 2px solid #ff9900;
            padding-bottom: 30px;
        }
        h1 {
            color: #ff9900;
            font-size: 3.5em;
            text-shadow: 0 0 20px #ff9900, 0 0 40px rgba(255, 153, 0, 0.5);
            letter-spacing: 3px;
            margin-bottom: 10px;
        }
        .stardate {
            color: #ffff66;
            font-size: 1.1em;
            font-style: italic;
        }
        .primary-status {
            display: grid;
            grid-template-columns: 2fr 1fr 1fr;
            gap: 20px;
            margin-bottom: 40px;
        }
        .metrics-grid {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr 1fr;
            gap: 20px;
            margin-bottom: 40px;
        }
        .distribution-section {
            margin-bottom: 40px;
        }
        .card {
            background: #001f4d;
            border: 2px solid #ff9900;
            border-radius: 10px;
            padding: 25px;
            box-shadow: inset 0 0 15px rgba(255, 153, 0, 0.2);
        }
        .card-label {
            color: #ffff66;
            font-size: 0.8em;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 15px;
            opacity: 0.9;
        }
        .card-value {
            font-size: 2.5em;
            font-weight: bold;
            color: #ffcc00;
            margin-bottom: 10px;
        }
        .card-secondary {
            font-size: 0.9em;
            color: #99ff99;
            margin-bottom: 8px;
        }
        .status-bar-container {
            margin-top: 15px;
        }
        .status-bar {
            width: 100%;
            height: 20px;
            background: #000011;
            border: 1px solid #ff9900;
            border-radius: 3px;
            overflow: hidden;
        }
        .status-bar-fill {
            height: 100%;
            background: linear-gradient(90deg, #00ff00, #00cc00);
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 0.7em;
            color: #000000;
            font-weight: bold;
        }
        .status-bar-fill.warning {
            background: linear-gradient(90deg, #ffcc00, #ff9900);
        }
        .status-bar-fill.critical {
            background: linear-gradient(90deg, #ff3333, #cc0000);
        }
        .large-card {
            grid-column: 1 / 3;
        }
        .large-card .card-value {
            font-size: 4em;
        }
        .distribution-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
        }
        .tag-card {
            background: #001f4d;
            border: 1px solid #ff9900;
            border-radius: 5px;
            padding: 15px;
            text-align: center;
            font-size: 0.9em;
        }
        .tag-card .count {
            font-size: 1.8em;
            font-weight: bold;
            color: #00ff00;
            margin-bottom: 5px;
        }
        .tag-card .label {
            color: #ffff66;
            text-transform: uppercase;
            font-size: 0.75em;
            letter-spacing: 0.5px;
        }
        .test-table {
            width: 100%;
            border-collapse: collapse;
            margin: 40px 0;
            border: 2px solid #ff9900;
        }
        .test-table thead {
            background: linear-gradient(90deg, #003366, #004d99);
        }
        .test-table th {
            color: #ffff66;
            padding: 15px;
            text-align: left;
            text-transform: uppercase;
            letter-spacing: 1px;
            font-size: 0.95em;
            border-bottom: 2px solid #ff9900;
        }
        .test-table td {
            padding: 12px 15px;
            border-bottom: 1px solid rgba(255, 153, 0, 0.3);
        }
        .test-table tr:hover {
            background: rgba(255, 153, 0, 0.1);
        }
        .test-name { color: #ffcc00; font-weight: bold; }
        .test-passed { color: #00ff00; text-transform: uppercase; font-weight: bold; }
        .test-failed { color: #ff3333; text-transform: uppercase; font-weight: bold; }
        .footer {
            margin-top: 50px;
            padding-top: 30px;
            border-top: 2px solid #ff9900;
            text-align: center;
            color: #ffff66;
            font-size: 0.9em;
        }
        .footer-divider {
            color: #ff9900;
            margin: 0 10px;
        }
        .tag-section {
            margin: 30px 0;
            border: 2px solid #ff9900;
            border-radius: 5px;
            overflow: hidden;
        }
        .tag-section-header {
            background: linear-gradient(90deg, #003366, #004d99);
            padding: 15px 20px;
            cursor: pointer;
            display: flex;
            justify-content: space-between;
            align-items: center;
            color: #ffff66;
            text-transform: uppercase;
            letter-spacing: 1px;
            font-weight: bold;
            border-bottom: 2px solid #ff9900;
        }
        .tag-section-header:hover {
            background: linear-gradient(90deg, #004d99, #006699);
        }
        .tag-section-header .toggle-icon {
            font-size: 1.2em;
            transition: transform 0.3s ease;
        }
        .tag-section-header .toggle-icon.open {
            transform: rotate(180deg);
        }
        .tag-section-stats {
            padding: 10px 20px;
            background: rgba(0, 51, 102, 0.3);
            font-size: 0.9em;
            color: #99ff99;
            border-bottom: 1px solid rgba(255, 153, 0, 0.3);
        }
        .tag-section-content {
            display: none;
            padding: 0;
        }
        .tag-section-content.open {
            display: block;
        }
        .tag-test-table {
            width: 100%;
            border-collapse: collapse;
        }
        .tag-test-table tr:nth-child(even) {
            background: rgba(0, 0, 0, 0.2);
        }
        .tag-test-table td {
            padding: 10px 15px;
            border-bottom: 1px solid rgba(255, 153, 0, 0.2);
            color: #ffcc00;
        }
        .tag-test-table .test-passed {
            color: #00ff00;
        }
        .tag-test-table .test-failed {
            color: #ff3333;
        }
    </style>
</head>
<body>
    <div class="lcars-frame">
        <div class="lcars-header">
            <h1>LCARS NEXUS FINAL REPORT</h1>
            <div class="stardate">EOF
echo "            $(date '+%Y-%m-%d %H:%M:%S')" >> "$REPORT_PATH"
cat >> "$REPORT_PATH" << 'EOF'</div>
        </div>
        
        <!-- PRIMARY STATUS: System Integrity + Key Indicators -->
        <div class="primary-status">
            <div class="card large-card">
                <div class="card-label">System Integrity</div>
                <div class="card-value">EOF
echo "$INTEGRITY_PERCENT%" >> "$REPORT_PATH"
cat >> "$REPORT_PATH" << 'EOF'</div>
                <div class="status-bar-container">
                    <div class="status-bar">
                        <div class="status-bar-fill EOF
echo "$INTEGRITY_CLASS" >> "$REPORT_PATH"
echo "\" style=\"width: $INTEGRITY_PERCENT%;\"></div>" >> "$REPORT_PATH"
cat >> "$REPORT_PATH" << 'EOF'
                    </div>
                </div>
                <div class="card-secondary">EOF
echo "$PASSED_TESTS of $TOTAL_TESTS tests passed</div>" >> "$REPORT_PATH"
cat >> "$REPORT_PATH" << 'EOF'
            </div>
            <div class="card">
                <div class="card-label">Critical Systems</div>
                <div class="card-value">EOF
echo "$CRITICAL_TESTS</div>" >> "$REPORT_PATH"
cat >> "$REPORT_PATH" << 'EOF'
                <div class="card-secondary">Core systems validated</div>
                <div class="card-secondary">Status: Active</div>
            </div>
            <div class="card">
                <div class="card-label">Execution Status</div>
                <div class="card-value">Ready</div>
                <div class="card-secondary">All phases complete</div>
                <div class="card-secondary">Deployment ready</div>
            </div>
        </div>

        <!-- METRICS GRID: Performance, Regression, Memory, Frames -->
        <div class="metrics-grid">
            <div class="card">
                <div class="card-label">Performance Matrix</div>
                <div class="card-value">EOF
echo "$AVG_DURATION" >> "$REPORT_PATH"
echo "ms</div>" >> "$REPORT_PATH"
cat >> "$REPORT_PATH" << 'EOF'
                <div class="card-secondary">Avg duration</div>
                <div class="card-secondary">Good</div>
            </div>
            <div class="card">
                <div class="card-label">Regression Alert</div>
                <div class="card-value">EOF
echo "$REGRESSION_COUNT</div>" >> "$REPORT_PATH"
cat >> "$REPORT_PATH" << 'EOF'
                <div class="card-secondary">Tests slower than baseline</div>
                <div class="card-secondary">Investigate</div>
            </div>
            <div class="card">
                <div class="card-label">Memory Allocation</div>
                <div class="card-value">N/A</div>
                <div class="card-secondary">Peak usage tracking</div>
                <div class="card-secondary">Requires ArgusLens</div>
            </div>
            <div class="card">
                <div class="card-label">Frame Diagnostics</div>
                <div class="card-value">N/A</div>
                <div class="card-secondary">Game-thread performance</div>
                <div class="card-secondary">Requires PIE execution</div>
            </div>
        </div>

        <!-- TEST DISTRIBUTION BY TAG -->
        <div class="distribution-section">
            <div class="card-label" style="padding: 0 0 15px 0;">Test Distribution by Category</div>
            <div class="distribution-grid">
                <div class="tag-card"><div class="count">3</div><div class="label">Networking</div></div>
                <div class="tag-card"><div class="count">2</div><div class="label">Performance</div></div>
                <div class="tag-card"><div class="count">3</div><div class="label">Gameplay</div></div>
                <div class="tag-card"><div class="count">2</div><div class="label">Compliance</div></div>
                <div class="tag-card"><div class="count">2</div><div class="label">Integration</div></div>
                <div class="tag-card"><div class="count">2</div><div class="label">Stress</div></div>
                <div class="tag-card"><div class="count">1</div><div class="label">Editor</div></div>
                <div class="tag-card"><div class="count">2</div><div class="label">Rendering</div></div>
            </div>
        </div>

        <!-- FLAT TEST TABLE (For reference) -->
        <div style="margin-top: 60px; padding-top: 40px; border-top: 2px solid #ff9900;">
            <div class="card-label" style="margin-bottom: 20px;">Complete Test Listing</div>
            <table class="test-table">
            <thead>
                <tr>
                    <th style="width: 60%;">Test Name</th>
                    <th style="width: 40%;">Status</th>
                </tr>
            </thead>
            <tbody>
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
            </tbody>
            </table>
        </div>

        <div class="footer">
            Generated by NEXUS <span class="footer-divider">|</span> Palantir Observer <span class="footer-divider">|</span> Quantum Observer Network<br>
            <span style="margin-top: 15px; display: block; color: #ffcc00; font-style: italic;">May the data be with you.</span>
        </div>
    </div>
</body>
<script>
function toggleSection(headerElement) {
    const icon = headerElement.querySelector('.toggle-icon');
    const content = headerElement.nextElementSibling.nextElementSibling;
    
    if (content.classList.contains('open')) {
        content.classList.remove('open');
        icon.classList.remove('open');
    } else {
        content.classList.add('open');
        icon.classList.add('open');
    }
}

document.addEventListener('DOMContentLoaded', function() {
    document.querySelectorAll('.tag-section').forEach(section => {
        const content = section.querySelector('.tag-section-content');
        const hasFailures = content.querySelector('.test-failed');
        if (hasFailures) {
            content.classList.add('open');
            section.querySelector('.toggle-icon').classList.add('open');
        }
    });
});
</script>
</html>
EOF

echo "[SUCCESS] Report generated successfully!" 
echo "Location: $REPORT_PATH"
echo ""

if [ "$OPEN_BROWSER" = true ]; then
    if command -v open &> /dev/null; then
        open "$REPORT_PATH"
    elif command -v xdg-open &> /dev/null; then
        xdg-open "$REPORT_PATH"
    fi
    echo "Opening in browser..."
else
    echo "To open in browser, run:"
    echo "   open '$REPORT_PATH'          (macOS)"
    echo "   xdg-open '$REPORT_PATH'      (Linux)"
fi

echo ""
echo "Enhanced LCARS dashboard features:"
echo "  - System Integrity card with status bar"
echo "  - 4-column metrics grid (Performance, Regression, Memory, Frames)"
echo "  - Test distribution by category"
echo "  - Complete flat test listing for reference"
        }
        .header-title h1 {
            color: #ff9900;
            font-size: 3em;
            letter-spacing: 3px;
            text-shadow: 0 0 20px #ff9900;
        }
        .subtitle { color: #ffcc00; font-size: 1.2em; margin-top: 10px; }
        .timestamp { color: #99ccff; font-size: 1.1em; margin-top: 15px; }
        .summary-section, .api-section, .performance-section, .details-section {
            background: #001133;
            padding: 30px;
            margin-bottom: 30px;
            border-radius: 10px;
        }
        .summary-section { border-left: 10px solid #ff9900; }
        .api-section { border-left: 10px solid #00ccff; }
        .performance-section { border-left: 10px solid #ff00ff; }
        .details-section { border-left: 10px solid #ffcc00; }
        .summary-section h2 { color: #ff9900; }
        .api-section h2 { color: #00ccff; }
        .performance-section h2 { color: #ff00ff; }
        .details-section h2 { color: #ffcc00; }
        h2 { font-size: 2em; margin-bottom: 20px; text-transform: uppercase; }
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }
        .stat-card {
            background: #000022;
            border: 2px solid #ffcc00;
            border-radius: 10px;
            padding: 20px;
            text-align: center;
        }
        .stat-card.passed { border-color: #00ff00; }
        .stat-card.failed { border-color: #ff0000; }
        .stat-label { font-size: 0.9em; color: #99ccff; margin-bottom: 10px; }
        .stat-value { font-size: 3em; font-weight: bold; color: #ffcc00; }
        .stat-card.passed .stat-value { color: #00ff00; }
        .stat-card.failed .stat-value { color: #ff0000; }
        .chart-container {
            background: #000022;
            border: 2px solid #ffcc00;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
        }
        .chart-title { color: #ff9900; font-size: 1.3em; margin-bottom: 15px; }
        .bar { background: #00ccff; margin: 5px 0; padding: 8px; border-radius: 3px; }
        .bar-label { color: #ffcc00; display: inline-block; width: 150px; }
        .bar-fill { background: linear-gradient(90deg, #00ccff, #00ffff); height: 20px; border-radius: 3px; display: inline-block; }
        .test-section {
            background: #000022;
            border: 2px solid #ffcc00;
            border-radius: 10px;
            padding: 20px;
            margin: 15px 0;
        }
        .test-success {
            border-left: 5px solid #00ff00;
            color: #00ff00;
            padding: 10px;
            margin: 5px 0;
            background: rgba(0, 255, 0, 0.1);
        }
        .test-error {
            border-left: 5px solid #ff0000;
            color: #ff0000;
            padding: 10px;
            margin: 5px 0;
            background: rgba(255, 0, 0, 0.1);
        }
        .test-warning {
            border-left: 5px solid #ffaa00;
            color: #ffaa00;
            padding: 10px;
            margin: 5px 0;
            background: rgba(255, 170, 0, 0.1);
        }
        .lcars-footer {
            background: #000033;
            border-top: 3px solid #ff9900;
            padding: 20px;
            margin-top: 40px;
            text-align: center;
            color: #ffcc00;
            border-radius: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="lcars-header">
            <div class="header-title">
                <h1>⊕ NEXUS ORCHESTRATION SUITE ⊕</h1>
                <div class="subtitle">LCARS Interface - API & Integration Analysis</div>
                <div class="timestamp">Generated: $(date +%Y-%m-%d\ %H:%M:%S)</div>
            </div>
        </div>

        <section class="summary-section">
            <h2>⌘ System Summary</h2>
            <div class="stats-grid">
                <div class="stat-card passed">
                    <div class="stat-label">Tests Passed</div>
                    <div class="stat-value">142</div>
                </div>
                <div class="stat-card failed">
                    <div class="stat-label">Tests Failed</div>
                    <div class="stat-value">3</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Success Rate</div>
                    <div class="stat-value">97.9%</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Execution Time</div>
                    <div class="stat-value">1m 24s</div>
                </div>
            </div>
        </section>

        <section class="api-section">
            <h2>▦ API Endpoints</h2>
            <div class="chart-container">
                <div class="chart-title">Response Times (avg ms)</div>
                <div class="bar">
                    <div class="bar-label">GET /users</div>
                    <div class="bar-fill" style="width: 45%;"></div> 45ms
                </div>
                <div class="bar">
                    <div class="bar-label">POST /auth/login</div>
                    <div class="bar-fill" style="width: 78%;"></div> 78ms
                </div>
                <div class="bar">
                    <div class="bar-label">GET /data/stream</div>
                    <div class="bar-fill" style="width: 125%;"></div> 125ms
                </div>
                <div class="bar">
                    <div class="bar-label">PUT /config/update</div>
                    <div class="bar-fill" style="width: 62%;"></div> 62ms
                </div>
            </div>
        </section>

        <section class="performance-section">
            <h2>◆ Performance Metrics</h2>
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-label">Avg Response Time</div>
                    <div class="stat-value">77ms</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Peak Load</div>
                    <div class="stat-value">2.3K req/s</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Memory Usage</div>
                    <div class="stat-value">256MB</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Uptime</div>
                    <div class="stat-value">99.97%</div>
                </div>
            </div>
        </section>

        <section class="details-section">
            <h2>◈ Detailed Results</h2>
            <div class="test-section">
                <div class="test-success">[OK] User authentication flow verified.</div>
                <div class="test-success">[OK] API rate limiting enforced correctly.</div>
                <div class="test-success">[OK] Data validation passed all checks.</div>
                <div class="test-warning">[WARN] Deprecated endpoint still active: /api/v1/legacy.</div>
                <div class="test-error">[ERROR] Cache synchronization failed on replica 3.</div>
                <div class="test-error">[ERROR] WebSocket connection timeout detected.</div>
                <div class="test-error">[ERROR] Authentication failed: Invalid or expired session token (PlayFab error 1074).</div>
            </div>
        </section>

        <footer class="lcars-footer">
            <p>Generated by: <strong>Nexus Orchestration Engine</strong> | Palantír Subsystem</p>
            <p>Framework: NexusQA v1.0 | © 2025</p>
        </footer>
    </div>
</body>
</html>
EOF

FILE_SIZE=$(stat -f%z "$REPORT_PATH" 2>/dev/null || stat -c%s "$REPORT_PATH")
FILE_SIZE_KB=$((FILE_SIZE / 1024))

echo "[SUCCESS] Report generated successfully!"
echo "Location: $REPORT_PATH"
echo "File size: ${FILE_SIZE_KB} KB"
echo ""
echo "Open in browser (macOS):"
echo "   open '$REPORT_PATH'"
echo ""
echo "Open in browser (Linux):"
echo "   xdg-open '$REPORT_PATH'"
echo ""
echo "Use this for screenshots!"
