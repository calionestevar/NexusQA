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
	TArray<FString> LocalEventLog;
	float LocalUptime = 0.0f;
	{
		FScopeLock _lock(&DashboardMutex);
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

	// Load HTML template
	FString TemplateDir = FPaths::ProjectSourceDir() / TEXT("FringeNetwork/Private");
	FString TemplatePath = TemplateDir / TEXT("ObserverNetworkDashboard.html");
	FString Html = TEXT("");
	
	if (!FFileHelper::LoadFileToString(Html, *TemplatePath))
	{
		UE_LOG(LogTemp, Error, TEXT("❌ OBSERVER NETWORK FAILED TO LOAD HTML TEMPLATE: %s"), *TemplatePath);
		return;
	}

	// Build event log HTML
	FString EventLogHtml = TEXT("");
	int32 StartIdx = FMath::Max(0, LocalEventLog.Num() - 50);
	for (int32 i = StartIdx; i < LocalEventLog.Num(); ++i)
	{
		const FString& Entry = LocalEventLog[i];
		FString Class = Entry.Contains(TEXT("BLOCKED")) ? TEXT("blocked") : TEXT("failed");
		EventLogHtml += TEXT("\t\t\t<div class=\"event ") + Class + TEXT("\">") + Entry + TEXT("</div>\n");
	}

	// Replace template placeholders
	Html.ReplaceInline(TEXT("{UPTIME}"), *FString::Printf(TEXT("%.1f"), LocalUptime));
	Html.ReplaceInline(TEXT("{TOTAL_EVENTS}"), *FString::Printf(TEXT("%d"), LocalEventLog.Num()));
	Html.ReplaceInline(TEXT("{BLOCKED_COUNT}"), *FString::Printf(TEXT("%d"), BlockedCount));
	Html.ReplaceInline(TEXT("{FAILED_COUNT}"), *FString::Printf(TEXT("%d"), FailedCount));
	Html.ReplaceInline(TEXT("{EVENT_LOG}"), *EventLogHtml);
	Html.ReplaceInline(TEXT("{TIMESTAMP}"), *FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S UTC")));

	FString Path = ReportDir / FString::Printf(TEXT("Observer_Report_%s.html"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FFileHelper::SaveStringToFile(Html, *Path);
	UE_LOG(LogTemp, Warning, TEXT("📊 OBSERVER FINAL REPORT → %s"), *Path);
}