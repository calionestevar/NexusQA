#include "ObserverNetworkDashboard.h"

/**
 * Embedded HTML template for ObserverNetworkDashboard
 * Formatted for readability - minification happens at build time if needed
 * This serves as fallback when template file cannot be loaded from disk
 */
FString UObserverNetworkDashboard::GetEmbeddedHTMLTemplate()
{
	return R"(<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width,initial-scale=1.0">
	<title>OBSERVER NETWORK — Dimensional Stability Report</title>
	<style>
		@import url('https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;700');
		* { margin: 0; padding: 0; box-sizing: border-box; }
		body {
			background: linear-gradient(135deg, #0a0e27 0%, #1a1a3a 50%, #0f1428 100%);
			background-attachment: fixed;
			color: #e8d100;
			font-family: 'IBM Plex Mono', 'Courier New', monospace;
			padding: 40px 20px;
			line-height: 1.7;
			position: relative;
			overflow-x: hidden;
		}
		body::before {
			content: '';
			position: fixed;
			top: 0; left: 0;
			width: 100%; height: 100%;
			background: repeating-linear-gradient(0deg, rgba(232,209,0,0.03) 0, rgba(232,209,0,0.03) 2px, transparent 2px, transparent 4px);
			pointer-events: none;
			z-index: -1;
		}
		.container { max-width: 1000px; margin: 0 auto; position: relative; z-index: 1; }
		.warning-banner {
			background: linear-gradient(90deg, rgba(255,100,0,0.2) 0%, rgba(255,200,0,0.1) 100%);
			border: 2px solid #ff6400;
			border-radius: 3px;
			padding: 15px;
			margin-bottom: 30px;
			text-align: center;
			color: #ffaa00;
			font-weight: bold;
			text-shadow: 0 0 5px rgba(255,100,0,0.5);
		}
		h1 {
			color: #00ffff;
			text-align: center;
			margin-bottom: 5px;
			font-size: 2.8em;
			text-shadow: 0 0 20px #00ffff, 0 0 40px rgba(0,255,255,0.3);
			letter-spacing: 2px;
		}
		.header-info {
			text-align: center;
			margin-bottom: 30px;
			color: #ffaa00;
			font-weight: bold;
			font-size: 1.1em;
			text-shadow: 0 0 10px rgba(255,170,0,0.5);
		}
		.dimension-status {
			text-align: center;
			color: #00ff99;
			margin-bottom: 20px;
			font-size: 0.95em;
		}
		.stats-grid {
			display: grid;
			grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
			gap: 20px;
			margin-bottom: 30px;
		}
		.stat-box {
			background: linear-gradient(135deg, rgba(0,100,100,0.3) 0%, rgba(0,200,200,0.1) 100%);
			border: 2px solid #00ffff;
			padding: 20px;
			border-radius: 3px;
			text-align: center;
			box-shadow: inset 0 0 10px rgba(0,255,255,0.1), 0 0 15px rgba(0,255,255,0.2);
		}
		.stat-value {
			font-size: 2.5em;
			color: #00ffff;
			font-weight: bold;
			text-shadow: 0 0 10px #00ffff;
		}
		.stat-label {
			color: #ffaa00;
			font-size: 0.85em;
			margin-top: 8px;
			text-transform: uppercase;
			letter-spacing: 1px;
		}
		.events-section {
			background: linear-gradient(135deg, rgba(0,30,60,0.6) 0%, rgba(0,60,80,0.3) 100%);
			border: 2px solid #ff6400;
			border-radius: 3px;
			padding: 25px;
			margin-top: 30px;
			box-shadow: 0 0 20px rgba(255,100,0,0.15);
		}
		.events-section h2 {
			color: #ff6400;
			margin-bottom: 15px;
			border-bottom: 2px solid #ff6400;
			padding-bottom: 10px;
			text-transform: uppercase;
			letter-spacing: 1px;
			text-shadow: 0 0 10px rgba(255,100,0,0.3);
		}
		.event {
			margin: 12px 0;
			padding: 12px 15px;
			background: rgba(10,20,40,0.8);
			border-left: 4px solid #ffaa00;
			border-radius: 2px;
			font-size: 0.9em;
			transition: all 0.2s ease;
		}
		.event:hover {
			background: rgba(20,40,80,0.9);
			border-left-width: 6px;
		}
		.event.blocked {
			border-left-color: #00ff00;
			background: rgba(0,40,0,0.4);
		}
		.event.blocked::before {
			content: "⬤ REALITY STABLE — ";
			color: #00ff00;
			font-weight: bold;
		}
		.event.failed {
			border-left-color: #ff3333;
			background: rgba(60,0,0,0.4);
		}
		.event.failed::before {
			content: "⚠ ANOMALY DETECTED — ";
			color: #ff3333;
			font-weight: bold;
		}
		.footer {
			margin-top: 40px;
			text-align: center;
			color: #666;
			font-size: 0.85em;
			border-top: 1px solid rgba(232,209,0,0.2);
			padding-top: 20px;
		}
		.footer-text {
			color: #888;
			margin: 5px 0;
		}
		.framework-credit {
			color: #ffaa00;
			font-weight: bold;
			text-shadow: 0 0 5px rgba(255,170,0,0.3);
		}
	</style>
</head>
<body>
	<div class="container">
		<h1>⚛ OBSERVER NETWORK</h1>
		<div class="warning-banner">⬤ DIMENSIONAL STABILITY AUDIT REPORT</div>
		<div class="dimension-status">Reality Tears Detected: Scanning Universe Integrity...</div>
		<div class="stats-grid">
			<div class="stat-box">
				<div class="stat-value">{UPTIME}</div>
				<div class="stat-label">Observation Window (sec)</div>
			</div>
			<div class="stat-box">
				<div class="stat-value">{TOTAL_EVENTS}</div>
				<div class="stat-label">Reality Disruptions</div>
			</div>
			<div class="stat-box">
				<div class="stat-value" style="color:#00ff00;text-shadow:0 0 10px #00ff00">{BLOCKED_COUNT}</div>
				<div class="stat-label">Anomalies Neutralized</div>
			</div>
			<div class="stat-box">
				<div class="stat-value" style="color:#ff3333;text-shadow:0 0 10px #ff3333">{FAILED_COUNT}</div>
				<div class="stat-label">Uncontained Breaches</div>
			</div>
		</div>
		<div class="events-section">
			<h2>⬥ Dimensional Breach Log (Last 50 Events)</h2>
			{EVENT_LOG}
		</div>
		<div class="footer">
			<div class="footer-text">━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━</div>
			<div class="footer-text">Report Generated: {TIMESTAMP}</div>
			<div class="footer-text"><span class="framework-credit">NexusQA Framework — Observer Network Division</span></div>
			<div class="footer-text">Multi-Backend Reality Stabilization System</div>
			<div class="footer-text">━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━</div>
		</div>
	</div>
</body>
</html>)";
}

