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

void UObserverNetworkDashboard::Initialize()
{
    FScopeLock _lock(&DashboardMutex);
    SessionStartTime = FPlatformTime::Seconds();
    SafetyCounters.Empty();
    EventLog.Empty();
    UE_LOG(LogTemp, Warning, TEXT("OBSERVER NETWORK DASHBOARD ONLINE — WATCHING ALL REALITIES"));
}

void UObserverNetworkDashboard::LogSafetyEvent(const FString& EventType, const FString& Details)
{
    FScopeLock _lock(&DashboardMutex);
    SafetyCounters.FindOrAdd(EventType)++;
    FString Entry = FString::Printf(TEXT("[%8.2f] %s: %s"),
        FPlatformTime::Seconds() - SessionStartTime, *EventType, *Details);
    EventLog.Add(Entry);
    UE_LOG(LogTemp, Warning, TEXT("OBSERVER EVENT: %s"), *Entry);
}

void UObserverNetworkDashboard::UpdateLiveDashboard()
{
    if (!GEngine || !GEngine->GameViewport) return;
    // Copy data under lock so UI rendering doesn't hold the mutex while ImGui runs.
    TMap<FString, int32> LocalCounters;
    TArray<FString> LocalEventLog;
    float LocalUptime = 0.0f;
    {
        FScopeLock _lock(&DashboardMutex);
        LocalCounters = SafetyCounters;
        LocalEventLog = EventLog;
        LocalUptime = FPlatformTime::Seconds() - SessionStartTime;
    }

#if WITH_IMGUI
    ImGui::Begin("OBSERVER NETWORK — LIVE AUDIT", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "REALITY STATUS: STABLE");
    ImGui::Separator();

    ImGui::Text("Session Time: %.1f seconds", LocalUptime);
    ImGui::Separator();

    ImGui::Text("SAFETY EVENTS");
    for (const auto& Pair : LocalCounters)
    {
        FString Color = Pair.Key.Contains("BLOCKED") ? "[00ff00]" : "[ff3333]";
        ImGui::TextColored(ImVec4(1,1,0,1), "%s: %d", *Pair.Key, Pair.Value);
    }

    ImGui::Separator();
    ImGui::Text("RECENT EVENTS");
    for (int32 i = FMath::Max(0, LocalEventLog.Num() - 10); i < LocalEventLog.Num(); ++i)
    {
        ImGui::TextUnformatted(*LocalEventLog[i]);
    }

    ImGui::End();
#endif
}

void UObserverNetworkDashboard::GenerateWebReport()
{
    const FString ReportDir = FPaths::ProjectSavedDir() / TEXT("ObserverReports");
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*ReportDir);
    // Snapshot data under lock to avoid holding the lock while constructing/saving files.
    TArray<FString> LocalEventLog;
    float LocalUptime = 0.0f;
    {
        FScopeLock _lock(&DashboardMutex);
        LocalEventLog = EventLog;
        LocalUptime = FPlatformTime::Seconds() - SessionStartTime;
    }

    FString Html = R"(
<!DOCTYPE html>
<html><head><title>Observer Network Final Report</title>
<style>
    body { background:#000033; color:#ffcc00; font-family:monospace; padding:20px; }
    h1 { color:#ff9900; text-align:center; }
    .event { margin:10px 0; padding:10px; background:#001133; border-left:5px solid #ff9900; }
    .blocked { border-color:#00ff00; }
    .failed { border-color:#ff3333; }
</style></head><body>
<h1>OBSERVER NETWORK — FINAL REPORT</h1>
<p>Session Duration: )" + FString::SanitizeFloat(LocalUptime) + TEXT(R"( seconds</p>
)";

    for (const FString& Entry : LocalEventLog)
    {
        FString Class = Entry.Contains(TEXT("BLOCKED")) ? TEXT("blocked") : TEXT("failed");
        Html += TEXT("<div class=\"event ") + Class + TEXT("\">") + Entry + TEXT("</div>");
    }

    Html += TEXT("</body></html>");
    FString Path = ReportDir / FString::Printf(TEXT("Observer_Report_%s.html"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    FFileHelper::SaveStringToFile(Html, *Path);
    UE_LOG(LogTemp, Warning, TEXT("OBSERVER FINAL REPORT → %s"), *Path);
}