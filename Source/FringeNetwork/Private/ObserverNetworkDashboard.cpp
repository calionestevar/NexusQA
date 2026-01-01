#include "ObserverNetworkDashboard.h"
#if WITH_IMGUI
#include "imgui.h"
#endif
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"

static TMap<FString, int32> SafetyCounters;
static TArray<FString> EventLog;
static float SessionStartTime = 0.0f;
static FCriticalSection DashboardMutex;
static EDashboardBackend ActiveBackend = EDashboardBackend::Auto;

// Forward declaration for internal ImGui rendering
static void RenderImGuiDashboard(const TMap<FString, int32>& Counters, const TArray<FString>& Events, float Uptime);

void UObserverNetworkDashboard::Initialize(EDashboardBackend Backend)
{
	FScopeLock _lock(&DashboardMutex);
	SessionStartTime = FPlatformTime::Seconds();
	SafetyCounters.Empty();
	EventLog.Empty();
	
	// Determine actual backend to use
	ActiveBackend = Backend;
	if (Backend == EDashboardBackend::Auto)
	{
		// Try ImGui first, fallback to HTML
#if WITH_IMGUI
		ActiveBackend = EDashboardBackend::ImGui;
#else
		ActiveBackend = EDashboardBackend::HTMLOnly;
#endif
	}
	
	const TCHAR* BackendName = TEXT("Unknown");
	switch (ActiveBackend)
	{
		case EDashboardBackend::ImGui:
			BackendName = TEXT("ImGui");
			break;
		case EDashboardBackend::Slate:
			BackendName = TEXT("Slate");
			break;
		case EDashboardBackend::HTMLOnly:
			BackendName = TEXT("HTML Report Only");
			break;
		default:
			break;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("🔍 OBSERVER NETWORK DASHBOARD ONLINE [Backend: %s] — WATCHING ALL REALITIES"), BackendName);
}

void UObserverNetworkDashboard::LogSafetyEvent(const FString& EventType, const FString& Details)
{
	FScopeLock _lock(&DashboardMutex);
	SafetyCounters.FindOrAdd(EventType)++;
	FString Entry = FString::Printf(TEXT("[%8.2f] %s: %s"),
		FPlatformTime::Seconds() - SessionStartTime, *EventType, *Details);
	EventLog.Add(Entry);
	UE_LOG(LogTemp, Warning, TEXT("📍 OBSERVER EVENT: %s"), *Entry);
}

void UObserverNetworkDashboard::UpdateLiveDashboard()
{
	// Copy data under lock so UI rendering doesn't hold the mutex
	TMap<FString, int32> LocalCounters;
	TArray<FString> LocalEventLog;
	float LocalUptime = 0.0f;
	EDashboardBackend Backend;
	{
		FScopeLock _lock(&DashboardMutex);
		LocalCounters = SafetyCounters;
		LocalEventLog = EventLog;
		LocalUptime = FPlatformTime::Seconds() - SessionStartTime;
		Backend = ActiveBackend;
	}

	// Render based on active backend
	switch (Backend)
	{
		case EDashboardBackend::ImGui:
			RenderImGuiDashboard(LocalCounters, LocalEventLog, LocalUptime);
			break;

		case EDashboardBackend::Slate:
			// Slate rendering would go here
			// For now, just log that we'd render to Slate
			break;

		case EDashboardBackend::HTMLOnly:
			// HTMLOnly backend doesn't render live, only generates final report
			break;

		default:
			break;
	}
}

static void RenderImGuiDashboard(const TMap<FString, int32>& Counters, const TArray<FString>& Events, float Uptime)
{
#if WITH_IMGUI
	if (!GEngine || !GEngine->GameViewport) return;

	ImGui::Begin("🔍 OBSERVER NETWORK — LIVE AUDIT", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "REALITY STATUS: STABLE");
	ImGui::Separator();

	ImGui::Text("Session Time: %.1f seconds", Uptime);
	ImGui::Separator();

	ImGui::Text("SAFETY EVENTS");
	for (const auto& Pair : Counters)
	{
		bool bBlocked = Pair.Key.Contains(TEXT("BLOCKED"));
		ImVec4 Color = bBlocked ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0.33f, 0.33f, 1);
		ImGui::TextColored(Color, "%s: %d", *Pair.Key, Pair.Value);
	}

	ImGui::Separator();
	ImGui::Text("RECENT EVENTS (Last 10)");
	ImGui::BeginChild("EventLog", ImVec2(500, 200), true);
	for (int32 i = FMath::Max(0, Events.Num() - 10); i < Events.Num(); ++i)
	{
		ImGui::TextUnformatted(*Events[i]);
	}
	ImGui::EndChild();

	ImGui::End();
#endif
}

EDashboardBackend UObserverNetworkDashboard::GetActiveBackend()
{
	FScopeLock _lock(&DashboardMutex);
	return ActiveBackend;
}

void UObserverNetworkDashboard::GenerateWebReport()
{
	const FString ReportDir = FPaths::ProjectSavedDir() / TEXT("ObserverReports");
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*ReportDir);

	// Snapshot data under lock to avoid holding the lock while constructing/saving files
	TMap<FString, int32> LocalCounters;
	TArray<FString> LocalEventLog;
	float LocalUptime = 0.0f;
	{
		FScopeLock _lock(&DashboardMutex);
		LocalCounters = SafetyCounters;
		LocalEventLog = EventLog;
		LocalUptime = FPlatformTime::Seconds() - SessionStartTime;
	}

	// Count blocked vs failed events
	int32 BlockedCount = 0;
	int32 FailedCount = 0;
	for (const FString& Event : LocalEventLog)
	{
		if (Event.Contains(TEXT("BLOCKED")))
			BlockedCount++;
		else
			FailedCount++;
	}

	FString Html = R"(
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>OBSERVER NETWORK — Dimensional Stability Report</title>
	<style>
		@import url('https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;700&display=swap');
		
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
			top: 0;
			left: 0;
			width: 100%;
			height: 100%;
			background: repeating-linear-gradient(
				0deg,
				rgba(232, 209, 0, 0.03) 0px,
				rgba(232, 209, 0, 0.03) 2px,
				transparent 2px,
				transparent 4px
			);
			pointer-events: none;
			z-index: -1;
		}
		
		.container { 
			max-width: 1000px; 
			margin: 0 auto; 
			position: relative;
			z-index: 1;
		}
		
		.warning-banner {
			background: linear-gradient(90deg, rgba(255, 100, 0, 0.2) 0%, rgba(255, 200, 0, 0.1) 100%);
			border: 2px solid #ff6400;
			border-radius: 3px;
			padding: 15px;
			margin-bottom: 30px;
			text-align: center;
			color: #ffaa00;
			font-weight: bold;
			text-shadow: 0 0 5px rgba(255, 100, 0, 0.5);
		}
		
		h1 {
			color: #00ffff;
			text-align: center;
			margin-bottom: 5px;
			font-size: 2.8em;
			text-shadow: 0 0 20px #00ffff, 0 0 40px rgba(0, 255, 255, 0.3);
			letter-spacing: 2px;
		}
		
		.header-info {
			text-align: center;
			margin-bottom: 30px;
			color: #ffaa00;
			font-weight: bold;
			font-size: 1.1em;
			text-shadow: 0 0 10px rgba(255, 170, 0, 0.5);
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
			background: linear-gradient(135deg, rgba(0, 100, 100, 0.3) 0%, rgba(0, 200, 200, 0.1) 100%);
			border: 2px solid #00ffff;
			padding: 20px;
			border-radius: 3px;
			text-align: center;
			box-shadow: inset 0 0 10px rgba(0, 255, 255, 0.1), 0 0 15px rgba(0, 255, 255, 0.2);
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
			background: linear-gradient(135deg, rgba(0, 30, 60, 0.6) 0%, rgba(0, 60, 80, 0.3) 100%);
			border: 2px solid #ff6400;
			border-radius: 3px;
			padding: 25px;
			margin-top: 30px;
			box-shadow: 0 0 20px rgba(255, 100, 0, 0.15);
		}
		
		.events-section h2 {
			color: #ff6400;
			margin-bottom: 15px;
			border-bottom: 2px solid #ff6400;
			padding-bottom: 10px;
			text-transform: uppercase;
			letter-spacing: 1px;
			text-shadow: 0 0 10px rgba(255, 100, 0, 0.3);
		}
		
		.event {
			margin: 12px 0;
			padding: 12px 15px;
			background: rgba(10, 20, 40, 0.8);
			border-left: 4px solid #ffaa00;
			border-radius: 2px;
			font-size: 0.9em;
			transition: all 0.2s ease;
		}
		
		.event:hover {
			background: rgba(20, 40, 80, 0.9);
			border-left-width: 6px;
		}
		
		.event.blocked {
			border-left-color: #00ff00;
			background: rgba(0, 40, 0, 0.4);
		}
		
		.event.blocked::before {
			content: "⬤ REALITY STABLE — ";
			color: #00ff00;
			font-weight: bold;
		}
		
		.event.failed {
			border-left-color: #ff3333;
			background: rgba(60, 0, 0, 0.4);
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
			border-top: 1px solid rgba(232, 209, 0, 0.2);
			padding-top: 20px;
		}
		
		.footer-text {
			color: #888;
			margin: 5px 0;
		}
		
		.framework-credit {
			color: #ffaa00;
			font-weight: bold;
			text-shadow: 0 0 5px rgba(255, 170, 0, 0.3);
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
				<div class="stat-value">)" + FString::Printf(TEXT("%.1f"), LocalUptime) + R"(</div>
				<div class="stat-label">Observation Window (sec)</div>
			</div>
			<div class="stat-box">
				<div class="stat-value">)" + FString::Printf(TEXT("%d"), LocalEventLog.Num()) + R"(</div>
				<div class="stat-label">Reality Disruptions</div>
			</div>
			<div class="stat-box">
				<div class="stat-value" style="color: #00ff00; text-shadow: 0 0 10px #00ff00;">)" + FString::Printf(TEXT("%d"), BlockedCount) + R"(</div>
				<div class="stat-label">Anomalies Neutralized</div>
			</div>
			<div class="stat-box">
				<div class="stat-value" style="color: #ff3333; text-shadow: 0 0 10px #ff3333;">)" + FString::Printf(TEXT("%d"), FailedCount) + R"(</div>
				<div class="stat-label">Uncontained Breaches</div>
			</div>
		</div>

		<div class="events-section">
			<h2>⬥ Dimensional Breach Log (Last 50 Events)</h2>
)";

	// Add events (last 50 for manageability)
	int32 StartIdx = FMath::Max(0, LocalEventLog.Num() - 50);
	for (int32 i = StartIdx; i < LocalEventLog.Num(); ++i)
	{
		const FString& Entry = LocalEventLog[i];
		FString Class = Entry.Contains(TEXT("BLOCKED")) ? TEXT("blocked") : TEXT("failed");
		Html += TEXT("\t\t\t<div class=\"event ") + Class + TEXT("\">") + Entry + TEXT("</div>\n");
	}

	Html += R"(
		</div>
		
		<div class="footer">
			<div class="footer-text">━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━</div>
			<div class="footer-text">Report Generated: )" + FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S UTC")) + R"(</div>
			<div class="footer-text"><span class="framework-credit">NexusQA Framework — Observer Network Division</span></div>
			<div class="footer-text">Multi-Backend Reality Stabilization System</div>
			<div class="footer-text">━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━</div>
		</div>
	</div>
</body>
</html>
)";

	FString Path = ReportDir / FString::Printf(TEXT("Observer_Report_%s.html"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FFileHelper::SaveStringToFile(Html, *Path);
	UE_LOG(LogTemp, Warning, TEXT("📊 OBSERVER FINAL REPORT → %s"), *Path);
}