#!/bin/bash
#
# generate-lcars-report.sh
# Generate Sample LCARS HTML Report for Screenshots
# Cross-platform compatible (macOS, Linux)
#

REPORT_PATH="TestReports/LCARS_Demo_Report.html"
REPORT_DIR=$(dirname "$REPORT_PATH")

# Create directory if it doesn't exist
mkdir -p "$REPORT_DIR"

echo "Generating sample LCARS report..."

# Generate HTML report
cat > "$REPORT_PATH" << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Nexus Demo Suite - API & Integration Tests</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #000033 0%, #001155 100%);
            color: #ffcc00;
            padding: 20px;
            min-height: 100vh;
        }
        .container { max-width: 1400px; margin: 0 auto; }
        .lcars-header {
            background: #000033;
            border: 3px solid #ff9900;
            border-radius: 20px;
            padding: 30px;
            margin-bottom: 30px;
            position: relative;
        }
        .lcars-header::before {
            content: '';
            position: absolute;
            top: -10px;
            left: 20px;
            width: 200px;
            height: 20px;
            background: #ff9900;
            border-radius: 10px;
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
