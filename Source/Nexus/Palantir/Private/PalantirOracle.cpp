#include "PalantirObserver.h"
#include "NexusCore.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DateTime.h"
#include "imgui.h"
#include "NexusTest.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"
// LCARS export (optional integration)
#include "LCARSReporter.h"
#include "Misc/AutomationTest.h"
#include "LCARSProvider.h"
#include "Misc/ConfigCacheIni.h"

// In-memory maps populated by OnTestStarted/OnTestFinished.
static TMap<FString, bool> GPalantirTestResults;
static TMap<FString, FDateTime> GPalantirTestStartTimes;
static TMap<FString, double> GPalantirTestDurations;
// Support multiple artifacts per test (screenshots, logs, replays)
static TMap<FString, TArray<FString>> GPalantirArtifactPaths;
static FCriticalSection GPalantirMutex;

// Pluggable provider (set during Initialize)
static TUniquePtr<ILCARSResultsProvider> GLCARSProvider;

// Palantír-backed provider implementation (accesses the static maps above)
class FPalantirInMemoryProvider : public ILCARSResultsProvider
{
public:
    virtual FLCARSResults GetResults() override
    {
        FLCARSResults Out;
        FScopeLock _lock(&GPalantirMutex);
        // Copy boolean results
        for (const auto& P : GPalantirTestResults)
        {
            Out.Results.Add(P.Key, P.Value);
        }
        // Copy durations (convert stored double)
        for (const auto& P : GPalantirTestDurations)
        {
            Out.Durations.Add(P.Key, P.Value);
        }
        // Copy artifacts
        for (const auto& P : GPalantirArtifactPaths)
        {
            Out.Artifacts.Add(P.Key, P.Value);
        }
        return Out;
    }
};

// AutomationFramework provider: best-effort mapping from AutomationTestFramework
class FAutomationFrameworkProvider : public ILCARSResultsProvider
{
public:
    virtual FLCARSResults GetResults() override
    {
        FLCARSResults Out;
        const FAutomationTestFramework& Framework = FAutomationTestFramework::GetInstance();
        for (const FAutomationTestInstance& Test : Framework.GetPassedTests())
        {
            Out.Results.Add(Test.GetTestName(), true);
        }
        for (const FAutomationTestInstance& Test : Framework.GetFailedTests())
        {
            Out.Results.Add(Test.GetTestName(), false);
            // If automation framework exposes durations or messages later we can add them here
        }
        return Out;
    }
};

void FPalantirObserver::Initialize()
{
    UE_LOG(LogTemp, Warning, TEXT("PALANTÍR ONLINE — OBSERVING ALL REALITIES"));
    // Select LCARS provider via config: section [/Script/Nexus.Palantir], key LCARSSource
    FString Source;
    if (GConfig)
    {
        if (!GConfig->GetString(TEXT("/Script/Nexus.Palantir"), TEXT("LCARSSource"), Source, GEngineIni))
        {
            Source.Empty();
        }
    }

    if (Source.Equals(TEXT("AutomationFramework"), ESearchCase::IgnoreCase))
    {
        GLCARSProvider.Reset(new FAutomationFrameworkProvider());
        UE_LOG(LogTemp, Display, TEXT("LCARS provider: AutomationFramework selected"));
    }
    else
    {
        // Default to Palantír in-memory provider
        GLCARSProvider.Reset(new FPalantirInMemoryProvider());
        UE_LOG(LogTemp, Display, TEXT("LCARS provider: Palantír (in-memory) selected"));
    }
}

void FPalantirObserver::OnTestStarted(const FString& Name)
{
    UE_LOG(LogTemp, Display, TEXT("Palantír: Test started: %s"), *Name);
    UNexusCore::NotifyTestStarted(Name);
    // Record start time for duration measurement
    GPalantirTestStartTimes.Add(Name, FDateTime::Now());
}

void FPalantirObserver::RegisterArtifact(const FString& TestName, const FString& ArtifactPath)
{
    FScopeLock _lock(&GPalantirMutex);
    if (!GPalantirArtifactPaths.Contains(TestName))
    {
        GPalantirArtifactPaths.Add(TestName, TArray<FString>());
    }
    GPalantirArtifactPaths[TestName].Add(ArtifactPath);
    UE_LOG(LogTemp, Display, TEXT("Palantír: Registered artifact for %s -> %s"), *TestName, *ArtifactPath);
}

void FPalantirObserver::OnTestFinished(const FString& Name, bool bPassed)
{
    UE_LOG(LogTemp, Display, TEXT("Palantír: Test finished: %s -> %s"), *Name, bPassed ? TEXT("PASSED") : TEXT("FAILED"));
    UNexusCore::NotifyTestFinished(Name, bPassed);

    // Record the result for final reporting (JUnit, HTML)
    {
        FScopeLock _lock(&GPalantirMutex);
        GPalantirTestResults.Add(Name, bPassed);
    }

    // Compute duration if we recorded a start time
    double DurationSeconds = 0.0;
    {
        FScopeLock _lock(&GPalantirMutex);
        if (GPalantirTestStartTimes.Contains(Name))
        {
            FDateTime Start = GPalantirTestStartTimes[Name];
            DurationSeconds = (FDateTime::Now() - Start).GetTotalSeconds();
            GPalantirTestStartTimes.Remove(Name);
        }
    }
    {
        FScopeLock _lock(&GPalantirMutex);
        GPalantirTestDurations.Add(Name, DurationSeconds);
    }

    // Ensure report directory exists and write a per-test log/artifact (simple summary).
    const FString ReportDir = FPaths::ProjectSavedDir() / TEXT("NexusReports");
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*ReportDir);
    // Sanitize test name for filename
    FString SafeName = Name;
    for (TCHAR& C : SafeName) if (!FChar::IsAlnum(C)) C = TEXT('_');
    const FString TestLogPath = ReportDir / FString::Printf(TEXT("test_%s.log"), *SafeName);
    FString LogContents = FString::Printf(TEXT("Test: %s\nResult: %s\nDuration: %.3fs\nTime: %s\n"),
        *Name,
        bPassed ? TEXT("PASSED") : TEXT("FAILED"),
        DurationSeconds,
        *FDateTime::Now().ToString());
    FFileHelper::SaveStringToFile(LogContents, *TestLogPath);
    // Add the per-test log as an artifact; keep array for multiple artifacts.
    {
        FScopeLock _lock(&GPalantirMutex);
        if (!GPalantirArtifactPaths.Contains(Name))
        {
            GPalantirArtifactPaths.Add(Name, TArray<FString>());
        }
        GPalantirArtifactPaths[Name].Add(TestLogPath);
    }

    if (!bPassed)
    {
        // Find the test to determine its priority; if critical, signal abort
        for (FNexusTest* Test : UNexusCore::DiscoveredTests)
        {
            if (Test && Test->TestName == Name)
            {
                if ((static_cast<uint8>(Test->Priority) & static_cast<uint8>(ETestPriority::Critical)) != 0)
                {
                    const FString Reason = FString::Printf(TEXT("Critical test failed: %s"), *Name);
                    UNexusCore::SignalAbort(Reason);
                }
                break;
            }
        }
    }
}

void FPalantirObserver::UpdateLiveOverlay()
{
    if (!GEngine || !GEngine->GameViewport) return;

    ImGui::Begin("PALANTÍR LIVE", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "NEXUS STATUS");
    ImGui::Separator();
    ImGui::Text("Tests Run: %d / %d", UNexusCore::PassedTests + UNexusCore::FailedTests, UNexusCore::TotalTests);
    ImGui::Text("Passed: %d", UNexusCore::PassedTests);
    ImGui::Text("Failed: %d", UNexusCore::FailedTests);
    ImGui::End();
}

void FPalantirObserver::GenerateFinalReport()
{
    const FString ReportDir = FPaths::ProjectSavedDir() / TEXT("NexusReports");
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*ReportDir);

    const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    const FString HtmlPath = ReportDir / FString::Printf(TEXT("LCARS_Report_%s.html"), *Timestamp);

    FString Html = TEXT(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>LCARS NEXUS FINAL REPORT</title>
    <style>
        body { background: #000033; color: #ffcc00; font-family: 'Courier New', monospace; margin: 40px; }
        .lcars { border: 4px solid #ff9900; border-radius: 20px; padding: 30px; max-width: 1000px; margin: auto; background: #000066; }
        h1 { color: #ff9900; text-align: center; font-size: 3.5em; text-shadow: 0 0 15px #ff9900; }
        .status { font-size: 2.2em; text-align: center; margin: 30px 0; }
        .passed { color: #00ff00; }
        .failed { color: #ff3333; }
        table { width: 100%; border-collapse: collapse; margin-top: 40px; }
        th, td { padding: 15px; text-align: left; border-bottom: 2px solid #ff9900; }
        th { background: #003366; color: #ffff66; }
        .footer { margin-top: 60px; text-align: center; color: #cccc00; }
    </style>
</head>
<body>
    <div class="lcars">
        <h1>LCARS NEXUS REPORT</h1>
        <div class="status passed">TOTAL TESTS: )" + FString::FromInt(UNexusCore::TotalTests) + TEXT(R"(</div>
        <div class="status passed">PASSED: )" + FString::FromInt(UNexusCore::PassedTests) + TEXT(R"(</div>
        <div class="status )" + (UNexusCore::FailedTests == 0 ? TEXT("passed") : TEXT("failed")) + TEXT(R"(")>FAILED: )" + FString::FromInt(UNexusCore::FailedTests) + TEXT(R"(</div>

        <table>
            <tr><th>Test Name</th><th>Status</th></tr>)");

    // Use recorded results from OnTestFinished (more accurate than index math)
    for (const auto& Pair : GPalantirTestResults)
    {
        const FString& TestName = Pair.Key;
        bool bPassed = Pair.Value;
        Html += FString::Printf(TEXT("<tr><td>%s</td><td class='%s'>%s</td></tr>"),
            *TestName,
            bPassed ? TEXT("passed") : TEXT("failed"),
            bPassed ? TEXT("PASSED") : TEXT("FAILED"));
    }

    Html += TEXT(R"(
        </table>
        <div class="footer">
            Generated by NEXUS • Palantír Observer • Stardate )" + FDateTime::Now().ToString() + TEXT(R"(
            <br>May the Great Link guide us.
        </div>
    </div>
</body>
</html>)");

    FFileHelper::SaveStringToFile(Html, *HtmlPath);
    UE_LOG(LogTemp, Warning, TEXT("LCARS FINAL REPORT GENERATED → %s"), *HtmlPath);

    // Emit a JUnit-style XML report for CI systems
    const int32 Total = GPalantirTestResults.Num();
    int32 Failures = 0;
    for (const auto& P : GPalantirTestResults) if (!P.Value) ++Failures;

    FString Xml = TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    Xml += FString::Printf(TEXT("<testsuites>\n  <testsuite name=\"Nexus\" tests=\"%d\" failures=\"%d\">\n"), Total, Failures);

    for (const auto& P : GPalantirTestResults)
    {
        const FString& TestName = P.Key;
        bool bPassed = P.Value;
        // Include duration if available
        double DurationSeconds = 0.0;
        if (GPalantirTestDurations.Contains(TestName))
        {
            DurationSeconds = GPalantirTestDurations[TestName];
        }
        // Check for artifact paths and include them in system-out
        FString SystemOut;
        if (GPalantirArtifactPaths.Contains(TestName))
        {
            const TArray<FString>& Arr = GPalantirArtifactPaths[TestName];
            for (const FString& P : Arr)
            {
                SystemOut += FString::Printf(TEXT("%s\n"), *P);
            }
        }

        Xml += FString::Printf(TEXT("    <testcase classname=\"NexusTests\" name=\"%s\" time=\"%.3f\">"), *TestName, DurationSeconds);
        if (!bPassed)
        {
            Xml += TEXT("\n      <failure message=\"failed\">Test failed</failure>\n");
        }
        if (!SystemOut.IsEmpty())
        {
            Xml += FString::Printf(TEXT("      <system-out><![CDATA[%s]]></system-out>\n"), *SystemOut);
        }
        Xml += TEXT("    </testcase>\n");
    }

    Xml += TEXT("  </testsuite>\n</testsuites>\n");

    const FString XmlPath = ReportDir / TEXT("nexus-results.xml");
    if (FFileHelper::SaveStringToFile(Xml, *XmlPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("JUnit XML report written → %s"), *XmlPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to write JUnit XML report → %s"), *XmlPath);
    }

    // Optionally export LCARS JSON via configured provider and register as an artifact
    {
        const FString LcarsPath = ReportDir / TEXT("LCARSReport.json");
        FLCARSResults Results;
        if (GLCARSProvider.IsValid())
        {
            Results = GLCARSProvider->GetResults();
        }
        else
        {
            // Fallback: copy the in-memory Palantír maps
            FScopeLock _lock(&GPalantirMutex);
            for (const auto& P : GPalantirTestResults) Results.Results.Add(P.Key, P.Value);
            for (const auto& P : GPalantirTestDurations) Results.Durations.Add(P.Key, P.Value);
            for (const auto& P : GPalantirArtifactPaths) Results.Artifacts.Add(P.Key, P.Value);
        }

        LCARSReporter::ExportResultsToLCARSFromPalantir(Results.Results, Results.Durations, Results.Artifacts, LcarsPath);
        // Register with Palantír so CI and artifact collectors pick it up
        FPalantirObserver::RegisterArtifact(TEXT("LCARS_Final"), LcarsPath);
    }
}