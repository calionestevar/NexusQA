#include "ObserverNetworkDashboard.h"
#if WITH_IMGUI
#include "imgui.h"
#endif
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"
#include "Engine/Engine.h"
#include "Widgets/SWindow.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"

static TMap<FString, int32> SafetyCounters;
static TArray<FString> EventLog;
static float SessionStartTime = 0.0f;
static FCriticalSection DashboardMutex;
static EDashboardBackend ActiveBackend = EDashboardBackend::Auto;

// Forward declarations for internal rendering backends
static void RenderImGuiDashboard(const TMap<FString, int32>& Counters, const TArray<FString>& Events, float Uptime);
static void RenderSlateDashboard(const TMap<FString, int32>& Counters, const TArray<FString>& Events, float Uptime);

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
	
	UE_LOG(LogTemp, Warning, TEXT("[INFO] OBSERVER NETWORK DASHBOARD ONLINE [Backend: %s] -- WATCHING ALL REALITIES"), BackendName);
}

void UObserverNetworkDashboard::LogSafetyEvent(const FString& EventType, const FString& Details)
{
	FScopeLock _lock(&DashboardMutex);
	SafetyCounters.FindOrAdd(EventType)++;
	FString Entry = FString::Printf(TEXT("[%8.2f] %s: %s"),
		FPlatformTime::Seconds() - SessionStartTime, *EventType, *Details);
	EventLog.Add(Entry);
	UE_LOG(LogTemp, Warning, TEXT("[EVENT] OBSERVER: %s"), *Entry);
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
			RenderSlateDashboard(LocalCounters, LocalEventLog, LocalUptime);
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

	ImGui::Begin("[OBSERVER] NETWORK LIVE AUDIT", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
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

static void RenderSlateDashboard(const TMap<FString, int32>& Counters, const TArray<FString>& Events, float Uptime)
{
	// Create or update Slate dashboard window for live monitoring
	static TWeakPtr<SWindow> DashboardWindowPtr;
	static TSharedPtr<SVerticalBox> DashboardContentPtr;
	static uint32 FrameCounter = 0;
	
	// Create window if it doesn't exist or was closed
	if (!DashboardWindowPtr.IsValid())
	{
		if (!FSlateApplication::IsInitialized()) return;
		
		// Create the content widget
		TSharedPtr<SVerticalBox> ContentBox;
		SAssignNew(ContentBox, SVerticalBox);
		DashboardContentPtr = ContentBox;
		
		// Create main window
		TSharedPtr<SWindow> Window = SNew(SWindow)
			.Title(FText::FromString(TEXT("Observer Network Dashboard")))
			.ClientSize(FVector2D(600, 500))
			.SupportsMinimize(true)
			.SupportsMaximize(true)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(10)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("OBSERVER NETWORK — Real-Time Monitoring")))
					.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
				]
				+ SVerticalBox::Slot()
				.Padding(10)
				.FillHeight(1.0f)
				[
					ContentBox.ToSharedRef()
				]
			];
		
		FSlateApplication::Get().AddWindow(Window.ToSharedRef());
		DashboardWindowPtr = Window;
	}
	
	// Update content every frame (instead of every 60 frames)
	if (DashboardContentPtr.IsValid())
	{
		DashboardContentPtr->ClearChildren();
		
		// Add uptime header
		DashboardContentPtr->AddSlot()
			.Padding(5)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Uptime: %.1f sec"), Uptime)))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
			];
		
		// Add safety counters section
		DashboardContentPtr->AddSlot()
			.Padding(5, 10, 5, 5)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Safety Counters")))
				.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
			];
		
		for (const auto& Pair : Counters)
		{
			FString CounterText = FString::Printf(TEXT("  %s: %d"), *Pair.Key, Pair.Value);
			DashboardContentPtr->AddSlot()
				.Padding(10, 2, 5, 2)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(CounterText))
					.Font(FAppStyle::GetFontStyle("SmallFont"))
				];
		}
		
		// Add recent events section
		DashboardContentPtr->AddSlot()
			.Padding(5, 10, 5, 5)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Recent Events")))
				.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
			];
		
		int32 StartIdx = FMath::Max(0, Events.Num() - 10);
		for (int32 i = StartIdx; i < Events.Num(); ++i)
		{
			FString EventText = FString::Printf(TEXT("  [%d] %s"), i, *Events[i]);
			DashboardContentPtr->AddSlot()
				.Padding(10, 2, 5, 2)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(EventText))
					.Font(FAppStyle::GetFontStyle("SmallFont"))
					.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				];
		}
	}
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

	// Load HTML template (tries multiple paths for dev and packaged builds)
	FString Html = LoadHTMLTemplate();
	if (Html.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[FAIL] OBSERVER NETWORK FAILED TO LOAD HTML TEMPLATE"));
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
	UE_LOG(LogTemp, Warning, TEXT("[INFO] OBSERVER FINAL REPORT --> %s"), *Path);
}

FString UObserverNetworkDashboard::LoadHTMLTemplate()
{
	// Try loading from source directory first (development builds)
	FString SourceTemplatePath = FPaths::ProjectDir() / TEXT("Plugins/NexusQA/Source/FringeNetwork/Private/ObserverNetworkDashboard.html");
	FString Html;
	
	if (FFileHelper::LoadFileToString(Html, *SourceTemplatePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OK] OBSERVER TEMPLATE LOADED FROM SOURCE: %s"), *SourceTemplatePath);
		return Html;
	}

	// Try alternate source path (if plugin is in project directly)
	FString AltSourcePath = FPaths::ProjectDir() / TEXT("Source/FringeNetwork/Private/ObserverNetworkDashboard.html");
	if (FFileHelper::LoadFileToString(Html, *AltSourcePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OK] OBSERVER TEMPLATE LOADED FROM ALT SOURCE: %s"), *AltSourcePath);
		return Html;
	}

	// Try content directory (packaged builds may have files here)
	FString ContentPath = FPaths::ProjectContentDir() / TEXT("ObserverNetwork/ObserverNetworkDashboard.html");
	if (FFileHelper::LoadFileToString(Html, *ContentPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OK] OBSERVER TEMPLATE LOADED FROM CONTENT: %s"), *ContentPath);
		return Html;
	}

	// Fallback: use embedded template (works in all build configurations)
	UE_LOG(LogTemp, Warning, TEXT("[WARN] OBSERVER TEMPLATE NOT FOUND ON DISK, USING EMBEDDED FALLBACK"));
	return GetEmbeddedHTMLTemplate();
}