/**
 * LCARS Report HTML Template
 * Enhanced Starfleet-themed dashboard with metrics, test grouping, and collapsible sections
 */

#pragma once

const char* LCARS_REPORT_TEMPLATE = R"(<!DOCTYPE html>
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
            <div class="stardate">{STARDATE}</div>
        </div>
        
        <!-- PRIMARY STATUS: System Integrity + Key Indicators -->
        <div class="primary-status">
            <div class="card large-card">
                <div class="card-label">System Integrity</div>
                <div class="card-value">{INTEGRITY_PERCENT}%</div>
                <div class="status-bar-container">
                    <div class="status-bar">
                        <div class="status-bar-fill {INTEGRITY_CLASS}" style="width: {INTEGRITY_PERCENT}%;"></div>
                    </div>
                </div>
                <div class="card-secondary">{PASSED_TESTS} of {TOTAL_TESTS} tests passed</div>
            </div>
            <div class="card">
                <div class="card-label">Critical Systems</div>
                <div class="card-value">{CRITICAL_TESTS}</div>
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
                <div class="card-value">{AVG_DURATION}ms</div>
                <div class="card-secondary">Avg duration</div>
                <div class="card-secondary">{PERF_STATUS}</div>
            </div>
            <div class="card">
                <div class="card-label">Regression Alert</div>
                <div class="card-value">{REGRESSION_COUNT}</div>
                <div class="card-secondary">Tests slower than baseline</div>
                <div class="card-secondary">{REGRESSION_STATUS}</div>
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
                {TAG_DISTRIBUTION_CARDS}
            </div>
        </div>

        <!-- TEST RESULTS BY CATEGORY (COLLAPSIBLE) -->
        <div style="margin-top: 50px; margin-bottom: 40px;">
            <div class="card-label" style="padding: 0 0 15px 0;">Test Results by Category</div>
            {GROUPED_TEST_SECTIONS}
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
                {ALL_TESTS_TABLE_ROWS}
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
</html>)";
