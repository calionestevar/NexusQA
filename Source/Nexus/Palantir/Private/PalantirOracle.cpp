#include "PalantirOracle.h"
#include "NexusCore.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DateTime.h"

// Define WITH_IMGUI if not already defined by the build system
#ifndef WITH_IMGUI
#define WITH_IMGUI 0
#endif

#if WITH_IMGUI
#include "imgui.h"
#endif
#include "NexusTest.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"
// LCARS export (optional integration)
#include "LCARSReporter.h"
#include "Misc/AutomationTest.h"
#include "LCARSProvider.h"
#include "Misc/ConfigCacheIni.h"

// ============================================================================
// FPalantirOracle Implementation - Test Result Repository
// ============================================================================

FPalantirOracle& FPalantirOracle::Get()
{
	static FPalantirOracle Instance;
	return Instance;
}

void FPalantirOracle::RecordTestResult(const FString& TestName, const FPalantirTestResult& Result)
{
	FScopeLock Lock(&ResultsLock);
	TestResults.Add(TestName, Result);
}

const TMap<FString, FPalantirTestResult>& FPalantirOracle::GetAllTestResults() const
{
	FScopeLock Lock(&ResultsLock);
	return TestResults;
}

const FPalantirTestResult* FPalantirOracle::GetTestResult(const FString& TestName) const
{
	FScopeLock Lock(&ResultsLock);
	return TestResults.Find(TestName);
}

void FPalantirOracle::ClearAllResults()
{
	FScopeLock Lock(&ResultsLock);
	TestResults.Empty();
}

int32 FPalantirOracle::GetTotalTestCount() const
{
	FScopeLock Lock(&ResultsLock);
	return TestResults.Num();
}

int32 FPalantirOracle::GetPassedTestCount() const
{
	FScopeLock Lock(&ResultsLock);
	int32 Count = 0;
	for (const auto& Pair : TestResults)
	{
		if (Pair.Value.bPassed)
		{
			Count++;
		}
	}
	return Count;
}

int32 FPalantirOracle::GetFailedTestCount() const
{
	FScopeLock Lock(&ResultsLock);
	int32 Count = 0;
	for (const auto& Pair : TestResults)
	{
		if (!Pair.Value.bPassed)
		{
			Count++;
		}
	}
	return Count;
}

// ============================================================================
// End FPalantirOracle Implementation
// ============================================================================

// In-memory maps populated by OnTestStarted/OnTestFinished.
static TMap<FString, bool> GPalantirTestResults;
static TMap<FString, FDateTime> GPalantirTestStartTimes;
static TMap<FString, double> GPalantirTestDurations;
// Support multiple artifacts per test (screenshots, logs, replays)
static TMap<FString, TArray<FString>> GPalantirArtifactPaths;
// Store test metadata for report generation (tags, priority, etc.)
static TMap<FString, TArray<FString>> GPalantirTestTags;
static FCriticalSection GPalantirMutex;

// Pluggable provider (set during Initialize)
static TUniquePtr<ILCARSResultsProvider> GLCARSProvider;

// Palantir-backed provider implementation (accesses the static maps above)
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

// NexusCore provider: uses NexusQA framework's test tracking
// (replaces deprecated FAutomationTestFramework API which was removed in UE 5.6)
class FNexusCoreProvider : public ILCARSResultsProvider
{
public:
    virtual FLCARSResults GetResults() override
    {
        FLCARSResults Out;
        
        // Get results from FPalantirOracle singleton
        const TMap<FString, FPalantirTestResult>& OracleResults = FPalantirOracle::Get().GetAllTestResults();
        
        for (const auto& Pair : OracleResults)
        {
            const FString& TestName = Pair.Key;
            const FPalantirTestResult& TestResult = Pair.Value;
            
            // Add pass/fail result
            Out.Results.Add(TestName, TestResult.bPassed);
            
            // Add duration
            Out.Durations.Add(TestName, TestResult.Duration);
            
            // Add artifacts
            TArray<FString> Artifacts;
            if (!TestResult.ScreenshotPath.IsEmpty())
            {
                Artifacts.Add(TestResult.ScreenshotPath);
            }
            if (!TestResult.TraceFilePath.IsEmpty())
            {
                Artifacts.Add(TestResult.TraceFilePath);
            }
            if (!TestResult.LogFilePath.IsEmpty())
            {
                Artifacts.Add(TestResult.LogFilePath);
            }
            
            if (Artifacts.Num() > 0)
            {
                Out.Artifacts.Add(TestName, Artifacts);
            }
        }
        
        return Out;
    }
};

void FPalantirObserver::Initialize()
{
    UE_LOG(LogTemp, Warning, TEXT("PALANTIR ONLINE -- OBSERVING ALL REALITIES"));
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
        GLCARSProvider.Reset(new FNexusCoreProvider());
        UE_LOG(LogTemp, Display, TEXT("LCARS provider: AutomationFramework selected"));
    }
    else
    {
        // Default to Palantir in-memory provider
        GLCARSProvider.Reset(new FPalantirInMemoryProvider());
        UE_LOG(LogTemp, Display, TEXT("LCARS provider: Palantir (in-memory) selected"));
    }
}

void FPalantirObserver::OnTestStarted(const FString& Name)
{
    UE_LOG(LogTemp, Display, TEXT("Palantir: Test started: %s"), *Name);
    UNexusCore::NotifyTestStarted(Name);
    // Record start time for duration measurement
    {
        FScopeLock _lock(&GPalantirMutex);
        GPalantirTestStartTimes.Add(Name, FDateTime::Now());
    }
}

void FPalantirObserver::OnTestStarted(const FNexusTest* Test)
{
    if (!Test)
    {
        return;
    }
    
    UE_LOG(LogTemp, Display, TEXT("Palantir: Test started: %s"), *Test->TestName);
    UNexusCore::NotifyTestStarted(Test->TestName);
    
    // Record start time and metadata
    {
        FScopeLock _lock(&GPalantirMutex);
        GPalantirTestStartTimes.Add(Test->TestName, FDateTime::Now());
        
        // Store test tags for report generation
        if (Test->GetCustomTags().Num() > 0)
        {
            GPalantirTestTags.Add(Test->TestName, Test->GetCustomTags());
        }
    }
}

void FPalantirObserver::RegisterArtifact(const FString& TestName, const FString& ArtifactPath)
{
    FScopeLock _lock(&GPalantirMutex);
    if (!GPalantirArtifactPaths.Contains(TestName))
    {
        GPalantirArtifactPaths.Add(TestName, TArray<FString>());
    }
    GPalantirArtifactPaths[TestName].Add(ArtifactPath);
    UE_LOG(LogTemp, Display, TEXT("Palantir: Registered artifact for %s -> %s"), *TestName, *ArtifactPath);
}

void FPalantirObserver::OnTestFinished(const FString& Name, bool bPassed)
{
    UE_LOG(LogTemp, Display, TEXT("Palantir: Test finished: %s -> %s"), *Name, bPassed ? TEXT("PASSED") : TEXT("FAILED"));
    // NOTE: NotifyTestFinished is called by the caller (NexusCore), not here to avoid double-counting

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

void FPalantirObserver::OnTestSkipped(const FString& Name)
{
    UE_LOG(LogTemp, Warning, TEXT("Palantir: Test skipped: %s"), *Name);
    
    // Record skipped test result
    {
        FScopeLock _lock(&GPalantirMutex);
        GPalantirTestResults.Add(Name, false);  // Skipped counts as not-failed but not-passed
    }
    
    // Write per-test skip log
    const FString ReportDir = FPaths::ProjectSavedDir() / TEXT("NexusReports");
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*ReportDir);
    
    FString SafeName = Name;
    for (TCHAR& C : SafeName) if (!FChar::IsAlnum(C)) C = TEXT('_');
    const FString TestLogPath = ReportDir / FString::Printf(TEXT("test_%s.log"), *SafeName);
    FString LogContents = FString::Printf(TEXT("Test: %s\nResult: SKIPPED\nTime: %s\n"),
        *Name,
        *FDateTime::Now().ToString());
    FFileHelper::SaveStringToFile(LogContents, *TestLogPath);
    
    // Register skip log as artifact
    {
        FScopeLock _lock(&GPalantirMutex);
        if (!GPalantirArtifactPaths.Contains(Name))
        {
            GPalantirArtifactPaths.Add(Name, TArray<FString>());
        }
        GPalantirArtifactPaths[Name].Add(TestLogPath);
    }
}

void FPalantirObserver::UpdateLiveOverlay()
{
    if (!GEngine || !GEngine->GameViewport) return;

#if WITH_IMGUI
    ImGui::Begin("PALANTIR LIVE", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "NEXUS STATUS");
    ImGui::Separator();
    ImGui::Text("Tests Run: %d / %d", UNexusCore::PassedTests + UNexusCore::FailedTests, UNexusCore::TotalTests);
    ImGui::Text("Passed: %d", UNexusCore::PassedTests);
    ImGui::Text("Failed: %d", UNexusCore::FailedTests);
    ImGui::End();
#else
    // ImGui not available - overlay disabled
#endif
}

void FPalantirObserver::GenerateFinalReport()
{
    const FString ReportDir = FPaths::ProjectSavedDir() / TEXT("NexusReports");
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*ReportDir);

    const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    const FString HtmlPath = ReportDir / FString::Printf(TEXT("LCARS_Report_%s.html"), *Timestamp);

    // Start with the template (same pattern as UObserverNetworkDashboard)
    FString Html = LCARSReporter::GetEmbeddedHTMLTemplate();

    // Calculate system integrity percentage (passed / (passed + failed), excluding skipped)
    int32 ExecutedTests = UNexusCore::PassedTests + UNexusCore::FailedTests;
    double IntegrityPercent = (ExecutedTests > 0) ? 
        (static_cast<double>(UNexusCore::PassedTests) / ExecutedTests) * 100.0 : 0.0;
    
    FString IntegrityClass = IntegrityPercent < 70 ? TEXT("critical") : 
                             IntegrityPercent < 85 ? TEXT("warning") : TEXT("");
    
    // Replace placeholders with actual data
    Html.ReplaceInline(TEXT("{STARDATE}"), *FDateTime::Now().ToString());
    Html.ReplaceInline(TEXT("{INTEGRITY_PERCENT}"), *FString::Printf(TEXT("%.1f"), IntegrityPercent));
    Html.ReplaceInline(TEXT("{INTEGRITY_CLASS}"), *IntegrityClass);
    Html.ReplaceInline(TEXT("{PASSED_TESTS}"), *FString::FromInt(UNexusCore::PassedTests));
    Html.ReplaceInline(TEXT("{SKIPPED_TESTS}"), *FString::FromInt(UNexusCore::SkippedTests));
    Html.ReplaceInline(TEXT("{FAILED_TESTS}"), *FString::FromInt(UNexusCore::FailedTests));
    Html.ReplaceInline(TEXT("{TOTAL_TESTS}"), *FString::FromInt(UNexusCore::TotalTests));
    Html.ReplaceInline(TEXT("{CRITICAL_TESTS}"), *FString::FromInt(UNexusCore::CriticalTests));
    
    // Performance metrics
    double AvgDuration = UNexusCore::GetAverageTestDuration();
    FString PerfStatus = AvgDuration < 100 ? TEXT("Excellent") : 
                         AvgDuration < 200 ? TEXT("Good") : TEXT("Needs review");
    Html.ReplaceInline(TEXT("{AVG_DURATION}"), *FString::Printf(TEXT("%.0f"), AvgDuration));
    Html.ReplaceInline(TEXT("{PERF_STATUS}"), *PerfStatus);
    
    // Regression metrics
    int32 RegressionCount = UNexusCore::DetectRegressions(0);
    FString RegressionStatus = RegressionCount == 0 ? TEXT("All clear") : TEXT("Investigate");
    Html.ReplaceInline(TEXT("{REGRESSION_COUNT}"), *FString::FromInt(RegressionCount));
    Html.ReplaceInline(TEXT("{REGRESSION_STATUS}"), *RegressionStatus);
    
    // Generate tag distribution cards and grouped test sections
    // Collect unique tags dynamically from test results instead of hardcoding
    TArray<FString> UniqueTags;
    TMap<FString, int32> TagCountMap;
    TMap<FString, int32> TagPassCountMap;
    TMap<FString, TArray<FString>> TagTestsMap;
    
    // Iterate through actual test results and categorize by stored custom tags
    {
        FScopeLock _lock(&GPalantirMutex);
        for (const auto& ResultPair : GPalantirTestResults)
        {
            const FString& TestName = ResultPair.Key;
            bool bPassed = ResultPair.Value;
            
            // Get the stored tags for this test (captured during OnTestStarted)
            if (GPalantirTestTags.Contains(TestName))
            {
                const TArray<FString>& Tags = GPalantirTestTags[TestName];
                
                // Categorize by all custom tags
                for (const FString& Tag : Tags)
                {
                    if (!UniqueTags.Contains(Tag))
                    {
                        UniqueTags.Add(Tag);
                    }
                    
                    TagCountMap.FindOrAdd(Tag, 0)++;
                    if (bPassed) TagPassCountMap.FindOrAdd(Tag, 0)++;
                    TagTestsMap.FindOrAdd(Tag).Add(TestName);
                }
            }
        }
    }
    
    // Sort tags for consistent ordering
    UniqueTags.Sort();
    
    // Generate tag distribution cards
    FString TagCards;
    for (const FString& Tag : UniqueTags)
    {
        int32 TagCount = TagCountMap[Tag];
        TagCards += FString::Printf(
            TEXT("<div class=\"tag-card\">\n")
            TEXT("    <div class=\"count\">%d</div>\n")
            TEXT("    <div class=\"label\">%s</div>\n")
            TEXT("</div>\n"),
            TagCount,
            *Tag);
    }
    Html.ReplaceInline(TEXT("{TAG_DISTRIBUTION_CARDS}"), *TagCards);
    
    // Generate grouped test sections with actual categorized results
    FString GroupedSections;
    for (const FString& Tag : UniqueTags)
    {
        int32 TotalInTag = TagCountMap[Tag];
        int32 PassedInTag = TagPassCountMap[Tag];
        TArray<FString>& TestsInTag = TagTestsMap[Tag];
        
        FString PassPercent = TotalInTag > 0 ? 
            FString::Printf(TEXT("%.1f"), (static_cast<double>(PassedInTag) / TotalInTag) * 100.0) : 
            TEXT("0.0");
        
        GroupedSections += FString::Printf(
            TEXT("<div class=\"tag-section\">\n")
            TEXT("    <div class=\"tag-section-header\" onclick=\"toggleSection(this)\">\n")
            TEXT("        <span>%s Tests</span>\n")
            TEXT("        <span class=\"toggle-icon\">&#x25BC;</span>\n")
            TEXT("    </div>\n")
            TEXT("    <div class=\"tag-section-stats\">")
            TEXT("%d tests - %s%% passed</div>\n")
            TEXT("    <div class=\"tag-section-content\">\n")
            TEXT("        <table class=\"tag-test-table\">\n"),
            *Tag,
            TotalInTag,
            *PassPercent);
        
        // Add test rows for this tag
        {
            FScopeLock _lock(&GPalantirMutex);
            for (const FString& TestName : TestsInTag)
            {
                bool bPassed = GPalantirTestResults.Contains(TestName) && GPalantirTestResults[TestName];
                GroupedSections += FString::Printf(
                    TEXT("            <tr>\n")
                    TEXT("                <td class=\"%s\">%s</td>\n")
                    TEXT("            </tr>\n"),
                    bPassed ? TEXT("test-passed") : TEXT("test-failed"),
                    *TestName);
            }
        }
        
        GroupedSections += TEXT("        </table>\n    </div>\n</div>\n");
    }
    Html.ReplaceInline(TEXT("{GROUPED_TEST_SECTIONS}"), *GroupedSections);
    
    // Generate flat test table rows
    FString TableRows;
    for (const auto& Pair : GPalantirTestResults)
    {
        const FString& TestName = Pair.Key;
        bool bPassed = Pair.Value;
        TableRows += FString::Printf(
            TEXT("<tr><td class='test-name'>%s</td><td class='%s'>%s</td></tr>\n"),
            *TestName,
            bPassed ? TEXT("test-passed") : TEXT("test-failed"),
            bPassed ? TEXT("PASSED") : TEXT("FAILED"));
    }
    Html.ReplaceInline(TEXT("{ALL_TESTS_TABLE_ROWS}"), *TableRows);
    
    // Protect all file writes with mutex to prevent race conditions
    FScopeLock _lock(&GPalantirMutex);
    
    FFileHelper::SaveStringToFile(Html, *HtmlPath);
    UE_LOG(LogTemp, Warning, TEXT("LCARS FINAL REPORT GENERATED --> %s"), *HtmlPath);
    
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
            for (const FString& ArtifactPath : Arr)
            {
                SystemOut += FString::Printf(TEXT("%s\n"), *ArtifactPath);
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
        UE_LOG(LogTemp, Warning, TEXT("JUnit XML report written --> %s"), *XmlPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to write JUnit XML report --> %s"), *XmlPath);
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
            // Fallback: copy the in-memory Palantir maps
            FScopeLock _lock2(&GPalantirMutex);
            for (const auto& P : GPalantirTestResults) Results.Results.Add(P.Key, P.Value);
            for (const auto& P : GPalantirTestDurations) Results.Durations.Add(P.Key, P.Value);
            for (const auto& P : GPalantirArtifactPaths) Results.Artifacts.Add(P.Key, P.Value);
        }

        LCARSReporter::ExportResultsToLCARSFromPalantir(Results.Results, Results.Durations, Results.Artifacts, LcarsPath);
        // Register with Palantir so CI and artifact collectors pick it up
        FPalantirObserver::RegisterArtifact(TEXT("LCARS_Final"), LcarsPath);
    }
}