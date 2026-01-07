#include "ObserverNetworkDashboard.h"
#include "Nexus/Core/Public/NexusTest.h"
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
		
		// Create main window with improved sizing and positioning
		TSharedPtr<SWindow> Window = SNew(SWindow)
			.Title(FText::FromString(TEXT("Observer Network Dashboard")))
			.ClientSize(FVector2D(700, 600))
			.SupportsMinimize(true)
			.SupportsMaximize(true)
			.IsInitiallyMaximized(false)
			.SizingRule(ESizingRule::UserSized)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(10)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("📡 OBSERVER NETWORK — Real-Time Monitoring Dashboard")))
					.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.1f, 1.0f, 0.8f)))  // Cyan
				]
				+ SVerticalBox::Slot()
				.Padding(10)
				.FillHeight(1.0f)
				[
					ContentBox.ToSharedRef()
				]
			];
		
		FSlateApplication::Get().AddWindow(Window.ToSharedRef());
		
		// Bring window to front and focus it for visibility
		Window->BringToFront(true);
		FSlateApplication::Get().SetKeyboardFocus(Window);
		
		DashboardWindowPtr = Window;
		UE_LOG(LogTemp, Display, TEXT("📊 Observer Network Dashboard window created and focused"));
	}
	
	// Update content every frame (instead of every 60 frames)
	if (DashboardContentPtr.IsValid())
	{
		DashboardContentPtr->ClearChildren();
		
		// Add uptime header with emoji and formatting
		FString UptimeText = FString::Printf(TEXT("⏱️  Uptime: %.1f seconds | Frame: %u"), Uptime, FrameCounter++);
		DashboardContentPtr->AddSlot()
			.Padding(8)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(UptimeText))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FSlateColor(FLinearColor::Green))
			];
		
		// Add separator
		DashboardContentPtr->AddSlot()
			.Padding(5, 5, 5, 5)
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolBar.Background"))
				.Padding(0)
				[
					SNew(SBox)
					.HeightOverride(2.0f)
				]
			];
		
		// Add test status section with counters
		DashboardContentPtr->AddSlot()
			.Padding(8, 10, 8, 5)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("🧪 Test Status")))
				.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.0f, 0.8f)))  // Cyan
			];
		
		// Add pass/fail/skip counters from FNexusTest::AllResults
		int32 PassedCount = 0;
		int32 FailedCount = 0;
		int32 SkippedCount = 0;
		
		// Count test results from AllResults
		for (const FNexusTestResult& Result : FNexusTest::AllResults)
		{
			if (Result.bSkipped)
			{
				SkippedCount++;
			}
			else if (Result.bPassed)
			{
				PassedCount++;
			}
			else
			{
				FailedCount++;
			}
		}
		
		int32 TotalExecuted = PassedCount + FailedCount + SkippedCount;
		int32 TotalDiscovered = FNexusTest::AllTests.Num();
		
		FString PassedText = FString::Printf(TEXT("  ✓ Passed: %d"), PassedCount);
		FString FailedText = FString::Printf(TEXT("  ⚠️  Failed: %d"), FailedCount);
		FString SkippedText = FString::Printf(TEXT("  ⏭️  Skipped: %d"), SkippedCount);
		FString TotalText = FString::Printf(TEXT("  📊 Total: %d / %d"), TotalExecuted, TotalDiscovered);
		
		DashboardContentPtr->AddSlot()
			.Padding(12, 3, 8, 3)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(PassedText))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f)))  // Green
			];
		
		DashboardContentPtr->AddSlot()
			.Padding(12, 3, 8, 3)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(FailedText))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.3f, 0.3f)))  // Red
			];
		
		DashboardContentPtr->AddSlot()
			.Padding(12, 3, 8, 3)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(SkippedText))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f)))  // Gold
			];
		
		DashboardContentPtr->AddSlot()
			.Padding(12, 3, 8, 3)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TotalText))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))  // Gray
			];
		
		// Add separator
		DashboardContentPtr->AddSlot()
			.Padding(5, 8, 5, 5)
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolBar.Background"))
				.Padding(0)
				[
					SNew(SBox)
					.HeightOverride(2.0f)
				]
			];
		
		// Add safety counters section with formatting
		DashboardContentPtr->AddSlot()
			.Padding(8, 10, 8, 5)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("🛡️  Safety Counters")))
				.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.0f)))  // Gold
			];
		
		for (const auto& Pair : Counters)
		{
			// Color code counters based on value
			FLinearColor CounterColor = FLinearColor::White;
			if (Pair.Value > 10) CounterColor = FLinearColor(1.0f, 0.3f, 0.3f);  // Red for high counts
			else if (Pair.Value > 5) CounterColor = FLinearColor(1.0f, 0.7f, 0.0f);  // Orange for medium
			else if (Pair.Value > 0) CounterColor = FLinearColor(1.0f, 1.0f, 0.0f);  // Yellow for some
			
			FString CounterText = FString::Printf(TEXT("  📌 %s: %d"), *Pair.Key, Pair.Value);
			DashboardContentPtr->AddSlot()
				.Padding(12, 3, 8, 3)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(CounterText))
					.Font(FAppStyle::GetFontStyle("SmallFont"))
					.ColorAndOpacity(FSlateColor(CounterColor))
				];
		}
		
		// Add separator
		DashboardContentPtr->AddSlot()
			.Padding(5, 8, 5, 5)
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolBar.Background"))
				.Padding(0)
				[
					SNew(SBox)
					.HeightOverride(2.0f)
				]
			];
		
		// Add recent events section with better formatting
		FString EventHeaderText = FString::Printf(TEXT("📋 Recent Events (%d total)"), Events.Num());
		DashboardContentPtr->AddSlot()
			.Padding(8, 10, 8, 5)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(EventHeaderText))
				.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.8f, 1.0f)))  // Light blue
			];
		
		int32 StartIdx = FMath::Max(0, Events.Num() - 15);  // Show last 15 events
		for (int32 i = StartIdx; i < Events.Num(); ++i)
		{
			const FString& Event = Events[i];
			
			// Determine event type and color
			FLinearColor EventColor = FLinearColor::Yellow;
			FString EventIcon = TEXT("ℹ️");
			
			if (Event.Contains(TEXT("SKIPPED")))
			{
				EventColor = FLinearColor(1.0f, 0.8f, 0.0f);  // Gold for skipped
				EventIcon = TEXT("⏭️");
			}
			else if (Event.Contains(TEXT("BLOCKED")))
			{
				EventColor = FLinearColor(0.0f, 1.0f, 0.5f);  // Green for blocked (expected)
				EventIcon = TEXT("🛑");
			}
			else if (Event.Contains(TEXT("FAILED")))
			{
				EventColor = FLinearColor(1.0f, 0.3f, 0.3f);  // Red for failed (error)
				EventIcon = TEXT("⚠️");
			}
			else if (Event.Contains(TEXT("SUCCESS")))
			{
				EventColor = FLinearColor(0.0f, 1.0f, 0.0f);  // Green for success
				EventIcon = TEXT("✓");
			}
			
			FString EventText = FString::Printf(TEXT("  %s [%d] %s"), *EventIcon, i, *Event);
			DashboardContentPtr->AddSlot()
				.Padding(12, 2, 8, 2)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(EventText))
					.Font(FAppStyle::GetFontStyle("SmallFont"))
					.ColorAndOpacity(FSlateColor(EventColor))
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