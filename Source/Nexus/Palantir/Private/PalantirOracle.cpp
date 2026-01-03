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

    FString Html = R"(<!DOCTYPE html>
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
            max-width: 1200px;
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
        .status-row {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            gap: 20px;
            margin: 40px 0;
        }
        .status-card {
            background: #001f4d;
            border: 2px solid #ff9900;
            border-radius: 10px;
            padding: 25px;
            text-align: center;
            box-shadow: inset 0 0 15px rgba(255, 153, 0, 0.2);
        }
        .status-label {
            color: #ffff66;
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 10px;
        }
        .status-value {
            font-size: 2.5em;
            font-weight: bold;
        }
        .passed-value { color: #00ff00; }
        .failed-value { color: #ff3333; }
        .total-value { color: #ffcc00; }
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
    </style>
</head>
<body>
    <div class="lcars-frame">
        <div class="lcars-header">
            <h1>LCARS NEXUS FINAL REPORT</h1>
            <div class="stardate">Stardate )" + FDateTime::Now().ToString() + R"(</div>
        </div>
        
        <div class="status-row">
            <div class="status-card">
                <div class="status-label">Total Tests</div>
                <div class="status-value total-value">)" + FString::FromInt(UNexusCore::TotalTests) + R"(</div>
            </div>
            <div class="status-card">
                <div class="status-label">Passed</div>
                <div class="status-value passed-value">)" + FString::FromInt(UNexusCore::PassedTests) + R"(</div>
            </div>
            <div class="status-card">
                <div class="status-label">Failed</div>
                <div class="status-value )" + (UNexusCore::FailedTests == 0 ? TEXT("passed-value") : TEXT("failed-value")) + R"(">)" + FString::FromInt(UNexusCore::FailedTests) + R"(</div>
            </div>
        </div>
        
        <table class="test-table">
            <thead>
                <tr>
                    <th style="width: 60%;">Test Name</th>
                    <th style="width: 40%;">Status</th>
                </tr>
            </thead>
            <tbody>)";

    // Use recorded results from OnTestFinished (more accurate than index math)
    for (const auto& Pair : GPalantirTestResults)
    {
        const FString& TestName = Pair.Key;
        bool bPassed = Pair.Value;
        Html += FString::Printf(TEXT("<tr><td class='test-name'>%s</td><td class='%s'>%s</td></tr>\n"),
            *TestName,
            bPassed ? TEXT("test-passed") : TEXT("test-failed"),
            bPassed ? TEXT("PASSED") : TEXT("FAILED"));
    }

    Html += R"(            </tbody>
        </table>
        
        <div class="footer">
            Generated by NEXUS <span class="footer-divider">|</span> Palantir Observer <span class="footer-divider">|</span> Quantum Observer Network<br>
            <span style="margin-top: 15px; display: block; color: #ffcc00; font-style: italic;">May the data be with you.</span>
        </div>
    </div>
</body>
</html>)";

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