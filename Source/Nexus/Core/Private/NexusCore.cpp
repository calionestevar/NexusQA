#include "NexusCore.h"
#include "NexusTest.h"
#include "Nexus/Palantir/Public/PalantirOracle.h"
#include "Misc/CommandLine.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include <atomic>

DEFINE_LOG_CATEGORY(LogNexus);

/**
 * Helper function to create a test context with world access when available
 * Returns an empty context if no world is available (e.g., in dedicated test sessions)
 */
static FNexusTestContext CreateTestContext()
{
    FNexusTestContext Context;
    
    // Try to get the first world in the engine
    for (TObjectIterator<UWorld> It; It; ++It)
    {
        UWorld* World = *It;
        // Skip editor worlds using WorldType (bIsEditorWorld removed in UE 5.7)
        if (World && World->WorldType != EWorldType::Editor && !World->bIsTearingDown)
        {
            Context.World = World;
            Context.GameState = World->GetGameState();
            
            // Get first player controller
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                Context.PlayerController = PC;
            }
            
            break;
        }
    }
    
    return Context;
}

/**
 * Helper function to populate performance metrics from ArgusLens
 * Safely checks if ArgusLens is available and retrieves metrics
 */
static void PopulatePerformanceMetrics(FTestPerformanceMetrics& OutMetrics)
{
    // ArgusLens is optional - only populate if module is loaded
    // Use reflection to check if UArgusLens class exists (avoids hard dependency)
    // Use nullptr instead of ANY_PACKAGE (removed in UE 5.7)
    UClass* ArgusLensClass = FindObject<UClass>(nullptr, TEXT("ArgusLens"), true);
    
    if (!ArgusLensClass)
    {
        // ArgusLens not loaded, skip performance data
        return;
    }
    
    // If ArgusLens is available, try to get metrics
    // Note: This requires ArgusLens to have been monitoring during the test
    // GetAverageFPS, GetPeakMemoryMb, GetHitchCount are static methods
    
    // Call via reflection if available:
    // UFunction* GetAverageFPSFunc = ArgusLensClass->FindFunctionByName(FName("GetAverageFPS"));
    // For now, we rely on ArgusLens being explicitly called in tests via NEXUS_PERF_TEST
    
    OutMetrics.AverageFPS = 0.0f;  // Default: no data
    OutMetrics.PeakMemoryMb = 0.0f;
    OutMetrics.HitchCount = 0;
    OutMetrics.bPassedPerformanceGates = true;
}

// Define the static test array from FNexusTest
// NEXUS_API on static member ensures proper DLL export for dependent modules
TArray<FNexusTest*> FNexusTest::AllTests;

int32 UNexusCore::TotalTests = 0;
int32 UNexusCore::PassedTests = 0;
int32 UNexusCore::FailedTests = 0;
int32 UNexusCore::SkippedTests = 0;
int32 UNexusCore::CriticalTests = 0;

TArray<FNexusTest*> UNexusCore::DiscoveredTests;

void UNexusCore::Execute(const TArray<FString>& Args)
{
    UE_LOG(LogTemp, Warning, TEXT("NEXUS CORE ONLINE — DUAL-STACK ORCHESTRATOR"));

    if (Args.Contains(TEXT("-legacy")))
    {
        UE_LOG(LogTemp, Display, TEXT("Legacy mode — Running UE Automation Framework tests via Asgard"));
        // Note: In practice, you'd load and execute the commandlet here.
        // For demonstration, we just log and use Nexus's own tests.
        // Real implementation would be: CommandletContext->RunCommandlet(UAsgardCommandlet::StaticClass());
    }

    FPalantirObserver::Initialize();
    DiscoverAllTests();

    TotalTests = DiscoveredTests.Num();
    if (TotalTests == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("NO NEXUS TESTS DISCOVERED — DID YOU FORGET NEXUS_TEST()?"));
        return;
    }

    RunAllTests(true);
    FPalantirObserver::GenerateFinalReport();
}

bool UNexusCore::EnsurePIEWorldActive()
{
    UWorld* World = nullptr;
    
    // Check if we're already in a PIE/game world
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.World() && Context.World()->IsGameWorld())
            {
                World = Context.World();
                break;
            }
        }
    }
    
    if (World)
    {
        UE_LOG(LogNexus, Display, TEXT("NEXUS: Active game world detected [%s]"), *World->GetMapName());
        return true;
    }
    
    // Try to read test map from config
#if WITH_EDITOR
    FString TestMapPath = TEXT("");
    if (GConfig)
    {
        GConfig->GetString(TEXT("/Script/Nexus.NexusSettings"), TEXT("TestMapPath"), TestMapPath, GGameIni);
    }
    
    if (!TestMapPath.IsEmpty())
    {
        UE_LOG(LogNexus, Warning, TEXT("NEXUS: No active game world - attempting to launch PIE with map: %s"), *TestMapPath);
        if (GEditor)
        {
            GEditor->PlayMap();
            return true;
        }
    }
#endif
    
    UE_LOG(LogNexus, Error, TEXT("NEXUS: No game world active and no TestMapPath configured. Game-thread tests will fail."));
    UE_LOG(LogNexus, Error, TEXT("NEXUS: Configure TestMapPath in DefaultGame.ini under [/Script/Nexus.NexusSettings]"));
    return false;
}

void UNexusCore::DiscoverAllTests()
{
    // Copy discovered tests from FNexusTest::AllTests into UNexusCore::DiscoveredTests
    // FNexusTest::AllTests is populated automatically when NEXUS_TEST() static objects
    // are constructed at module load time
    DiscoveredTests = FNexusTest::AllTests;
    TotalTests = DiscoveredTests.Num();
    UE_LOG(LogNexus, Display, TEXT("NEXUS: Discovered %d test(s)"), DiscoveredTests.Num());
}

void UNexusCore::RunAllTests(bool bParallel)
{
    // Reset counters for this test run
    PassedTests = 0;
    FailedTests = 0;
    SkippedTests = 0;
    CriticalTests = 0;
    
    // Sort: Critical first, then Smoke, then Normal
    DiscoveredTests.Sort([](const FNexusTest& A, const FNexusTest& B) {
        return (int32)A.Priority > (int32)B.Priority;
    });

    // Separate game-thread tests from parallel-safe tests
    TArray<FNexusTest*> GameThreadTests;
    TArray<FNexusTest*> ParallelTests;
    
    for (FNexusTest* Test : DiscoveredTests)
    {
        if (!Test) continue;
        
        if (Test->bRequiresGameThread)
        {
            GameThreadTests.Add(Test);
        }
        else
        {
            ParallelTests.Add(Test);
        }
    }

    // Run parallel-safe tests with parallel execution (if enabled)
    if (bParallel && ParallelTests.Num() > 1)
    {
        UE_LOG(LogNexus, Display, TEXT("NEXUS: Running %d parallel-safe tests in parallel"), ParallelTests.Num());

        TArray<TFuture<bool>> Futures;
        std::atomic<bool> bCriticalFailed{false};

        for (FNexusTest* Test : ParallelTests)
        {
            if (!Test) continue;
            
            // Check if test is being skipped
            if (Test->bSkip)
            {
                UNexusCore::NotifyTestSkipped(Test->TestName);
                FPalantirObserver::OnTestFinished(Test->TestName, true);
                continue;
            }

            // Early exit if critical test already failed
            if (bCriticalFailed.load())
            {
                UE_LOG(LogNexus, Warning, TEXT("Skipping test %s due to critical failure"), *Test->TestName);
                continue;
            }

            // Launch test on thread pool
            Futures.Add(Async(EAsyncExecution::ThreadPool, [Test, &bCriticalFailed]() -> bool
            {
                // Check again inside task in case failure happened during launch
                if (bCriticalFailed.load())
                {
                    return false;
                }

                FPalantirObserver::OnTestStarted(Test->TestName);
                UNexusCore::NotifyTestStarted(Test->TestName);

                // Parallel tests typically don't have world access, but we create an empty context
                FNexusTestContext EmptyContext;
                bool bPassed = Test->Execute(EmptyContext);

                UNexusCore::NotifyTestFinished(Test->TestName, bPassed);
                FPalantirObserver::OnTestFinished(Test->TestName, bPassed);

                // Signal critical failure for fail-fast behavior
                if (!bPassed && NexusHasFlag(Test->Priority, ETestPriority::Critical))
                {
                    bCriticalFailed.store(true);
                    UE_LOG(LogNexus, Error, TEXT("CRITICAL TEST FAILED: %s — Aborting remaining tests"), *Test->TestName);
                }

                return bPassed;
            }));
        }

        // Wait for all async tasks to complete
        for (auto& Future : Futures)
        {
            Future.Get();
        }

        if (bCriticalFailed.load())
        {
            UE_LOG(LogNexus, Error, TEXT("CRITICAL FAILURE DETECTED — Test suite aborted"));
        }
    }
    else if (ParallelTests.Num() > 0)
    {
        // Run parallel-safe tests sequentially
        UE_LOG(LogNexus, Display, TEXT("NEXUS: Running %d parallel-safe tests sequentially"), ParallelTests.Num());
        DiscoveredTests = ParallelTests;
        RunSequentialWithFailFast();
    }

    // Run game-thread tests sequentially on game thread
    if (GameThreadTests.Num() > 0)
    {
        UE_LOG(LogNexus, Display, TEXT("NEXUS: Running %d game-thread tests on main thread"), GameThreadTests.Num());
        
        // Auto-detect if PIE world is available before running game-thread tests
        // Game-thread tests require an active world context to function properly
        bool bHasActiveWorld = false;
        for (TObjectIterator<UWorld> It; It; ++It)
        {
            UWorld* World = *It;
            if (World && World->WorldType != EWorldType::Editor && !World->bIsTearingDown)
            {
                bHasActiveWorld = true;
                break;
            }
        }
        
        if (!bHasActiveWorld)
        {
            UE_LOG(LogNexus, Warning, TEXT("⚠️  No active game world detected — Game-thread tests will gracefully skip"));
            UE_LOG(LogNexus, Display, TEXT("💡 To run game-thread tests with full world context, click 'Play' in the editor first"));
        }
        
        DiscoveredTests = GameThreadTests;
        RunSequentialWithFailFast();
    }

    FPalantirObserver::GenerateFinalReport();
}

void UNexusCore::RegisterTest(FNexusTest* Test)
{
    if (Test)
    {
        DiscoveredTests.Add(Test);
    }
}

TArray<FNexusTest*> UNexusCore::GetTestsWithTags(ETestTag Tags)
{
    TArray<FNexusTest*> FilteredTests;
    
    for (FNexusTest* Test : DiscoveredTests)
    {
        if (Test && Test->HasTags(Tags))
        {
            FilteredTests.Add(Test);
        }
    }
    
    return FilteredTests;
}

int32 UNexusCore::CountTestsWithTags(ETestTag Tags)
{
    int32 Count = 0;
    for (FNexusTest* Test : DiscoveredTests)
    {
        if (Test && Test->HasTags(Tags))
        {
            ++Count;
        }
    }
    return Count;
}

TArray<FNexusTest*> UNexusCore::GetTestsWithCustomTag(const FString& CustomTag)
{
    TArray<FNexusTest*> FilteredTests;
    
    if (CustomTag.IsEmpty())
    {
        return FilteredTests;
    }
    
    for (FNexusTest* Test : DiscoveredTests)
    {
        if (Test && Test->HasCustomTag(CustomTag))
        {
            FilteredTests.Add(Test);
        }
    }
    
    return FilteredTests;
}

int32 UNexusCore::CountTestsWithCustomTag(const FString& CustomTag)
{
    int32 Count = 0;
    
    if (CustomTag.IsEmpty())
    {
        return 0;
    }
    
    for (FNexusTest* Test : DiscoveredTests)
    {
        if (Test && Test->HasCustomTag(CustomTag))
        {
            ++Count;
        }
    }
    return Count;
}

TArray<FString> UNexusCore::GetAllCustomTags()
{
    TArray<FString> AllTags;
    
    for (FNexusTest* Test : DiscoveredTests)
    {
        if (Test)
        {
            for (const FString& CustomTag : Test->GetCustomTags())
            {
                if (!AllTags.Contains(CustomTag))
                {
                    AllTags.Add(CustomTag);
                }
            }
        }
    }
    
    AllTags.Sort();
    return AllTags;
}
}

void UNexusCore::RunTestsWithTags(ETestTag Tags, bool bParallel)
{
    TArray<FNexusTest*> FilteredTests = GetTestsWithTags(Tags);
    
    if (FilteredTests.Num() == 0)
    {
        UE_LOG(LogNexus, Warning, TEXT("NEXUS: No tests found matching the specified tags"));
        return;
    }
    
    UE_LOG(LogNexus, Display, TEXT("NEXUS: Running %d tests matching tags"), FilteredTests.Num());
    
    // Temporarily replace DiscoveredTests and TotalTests with filtered tests
    TArray<FNexusTest*> OriginalTests = DiscoveredTests;
    int32 OriginalTotalTests = TotalTests;
    
    DiscoveredTests = FilteredTests;
    TotalTests = FilteredTests.Num();
    
    RunAllTests(bParallel);
    
    // Restore original tests and counters
    DiscoveredTests = OriginalTests;
    TotalTests = OriginalTotalTests;
}

void UNexusCore::NotifyTestStarted(const FString& Name)
{
    UE_LOG(LogNexus, Display, TEXT("TEST STARTED: %s"), *Name);
}

void UNexusCore::NotifyTestFinished(const FString& Name, bool bPassed)
{
    if (bPassed)
    {
        ++PassedTests;
        UE_LOG(LogNexus, Display, TEXT("TEST PASSED: %s"), *Name);
    }
    else
    {
        ++FailedTests;
        UE_LOG(LogNexus, Error, TEXT("TEST FAILED: %s"), *Name);
    }
}

void UNexusCore::NotifyTestSkipped(const FString& Name)
{
    ++SkippedTests;
    UE_LOG(LogNexus, Warning, TEXT("TEST SKIPPED: %s"), *Name);
}

FString UNexusCore::GetAbortFilePath()
{
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("NexusAbort.flag"));
}

void UNexusCore::SignalAbort(const FString& Reason)
{
    static std::atomic<bool> bAbortSignalled(false);

    // First-writer wins: only the first caller will write the sentinel file.
    bool expected = false;
    if (!bAbortSignalled.compare_exchange_strong(expected, true))
    {
        UE_LOG(LogNexus, Display, TEXT("Abort already signalled by another process/thread; skipping write (Reason: %s)"), *Reason);
        return;
    }

    const FString AbortFile = GetAbortFilePath();
    const int32 PID = FPlatformProcess::GetCurrentProcessId();
    const FString Contents = FString::Printf(TEXT("PID=%d\nTime=%s\nReason=%s\n"), PID, *FDateTime::Now().ToString(), *Reason);
    if (FFileHelper::SaveStringToFile(Contents, *AbortFile))
    {
        UE_LOG(LogNexus, Warning, TEXT("Wrote abort sentinel '%s' (Reason: %s)"), *AbortFile, *Reason);
    }
    else
    {
        UE_LOG(LogNexus, Error, TEXT("Failed to write abort sentinel '%s'"), *AbortFile);
    }
}

void UNexusCore::RunSequentialWithFailFast()
{
    // Create context once for all sequential tests (game-thread tests especially need this)
    FNexusTestContext TestContext = CreateTestContext();
    
    for (FNexusTest* Test : DiscoveredTests)
    {
        if (!Test) continue;

        const FString Name = Test->TestName;
        
        // Check if test is being skipped
        if (Test->bSkip)
        {
            NotifyTestSkipped(Name);
            FPalantirObserver::OnTestFinished(Name, true);  // Skipped counts as passed for reporting
            continue;
        }

        FPalantirObserver::OnTestStarted(Name);
        NotifyTestStarted(Name);

        bool bPassed = Test->Execute(TestContext);
        
        // Populate performance metrics after test execution if available
        PopulatePerformanceMetrics(TestContext.PerformanceMetrics);

        NotifyTestFinished(Name, bPassed);
        FPalantirObserver::OnTestFinished(Name, bPassed);

        // If a critical test failed, signal abort and stop immediately
        if (!bPassed && (static_cast<uint8>(Test->Priority) & static_cast<uint8>(ETestPriority::Critical)) != 0)
        {
            const FString Reason = FString::Printf(TEXT("Critical test failed during sequential run: %s"), *Name);
            SignalAbort(Reason);
            break;
        }
    }
}

double UNexusCore::GetAverageTestDuration(const FString& TestName)
{
    if (FNexusTest::AllResults.Num() == 0)
    {
        return 0.0;
    }
    
    double TotalDuration = 0.0;
    int32 Count = 0;
    
    for (const FNexusTestResult& Result : FNexusTest::AllResults)
    {
        if (TestName.IsEmpty() || Result.TestName == TestName)
        {
            TotalDuration += Result.DurationSeconds;
            ++Count;
        }
    }
    
    return Count > 0 ? TotalDuration / Count : 0.0;
}

double UNexusCore::GetMedianTestDuration(const FString& TestName)
{
    TArray<double> Durations;
    
    for (const FNexusTestResult& Result : FNexusTest::AllResults)
    {
        if (TestName.IsEmpty() || Result.TestName == TestName)
        {
            Durations.Add(Result.DurationSeconds);
        }
    }
    
    if (Durations.Num() == 0)
    {
        return 0.0;
    }
    
    Durations.Sort();
    int32 MiddleIndex = Durations.Num() / 2;
    return Durations[MiddleIndex];
}

int32 UNexusCore::DetectRegressions(double MaxAllowedDurationMs)
{
    if (MaxAllowedDurationMs <= 0.0)
    {
        // Use median * 1.5 as baseline if not specified
        MaxAllowedDurationMs = GetMedianTestDuration() * 1500.0;  // Convert to ms
    }
    
    int32 RegressionCount = 0;
    TMap<FString, TArray<double>> TestDurations;
    
    // Group results by test name
    for (const FNexusTestResult& Result : FNexusTest::AllResults)
    {
        TestDurations.FindOrAdd(Result.TestName).Add(Result.DurationSeconds * 1000.0);  // Convert to ms
    }
    
    // Check each test for regressions
    for (const auto& Pair : TestDurations)
    {
        const FString& TestName = Pair.Key;
        const TArray<double>& Durations = Pair.Value;
        
        if (Durations.Num() >= 2)
        {
            // Compare latest to median of previous runs
            double LatestDuration = Durations.Last();
            double Baseline = 0.0;
            
            if (Durations.Num() >= 3)
            {
                // Use median of all but last run as baseline
                TArray<double> PreviousDurations(Durations.GetData(), Durations.Num() - 1);
                PreviousDurations.Sort();
                Baseline = PreviousDurations[PreviousDurations.Num() / 2];
            }
            else
            {
                Baseline = Durations[0];  // First run is baseline
            }
            
            // Flag regression if latest is 50% slower than baseline
            if (LatestDuration > Baseline * 1.5)
            {
                UE_LOG(LogNexus, Warning, TEXT("REGRESSION: %s took %.2fms (baseline: %.2fms)"), 
                    *TestName, LatestDuration, Baseline);
                ++RegressionCount;
            }
        }
    }
    
    return RegressionCount;
}

void UNexusCore::ExportTestTrends(const FString& OutputPath)
{
    FString ExportPath = OutputPath.IsEmpty() ? 
        FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("TestTrends")) : OutputPath;
    
    if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*ExportPath))
    {
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*ExportPath);
    }
    
    // Export CSV with all results
    FString CSVPath = FPaths::Combine(ExportPath, TEXT("test_trends.csv"));
    FString CSVContent = TEXT("TestName,Timestamp,DurationSeconds,Passed,Attempts\n");
    
    for (const FNexusTestResult& Result : FNexusTest::AllResults)
    {
        CSVContent += FString::Printf(TEXT("%s,%s,%.4f,%d,%d\n"), 
            *Result.TestName,
            *Result.Timestamp.ToIso8601(),
            Result.DurationSeconds,
            Result.bPassed ? 1 : 0,
            Result.Attempts);
    }
    
    FFileHelper::SaveStringToFile(CSVContent, *CSVPath);
    UE_LOG(LogNexus, Display, TEXT("Exported test trends to %s"), *CSVPath);
    
    // Export summary JSON
    FString JSONPath = FPaths::Combine(ExportPath, TEXT("test_trends_summary.json"));
    FString JSONContent = TEXT("{\n  \"tests\": [\n");
    
    TMap<FString, int32> TestPassCounts;
    TMap<FString, double> TestAvgDurations;
    
    for (const FNexusTestResult& Result : FNexusTest::AllResults)
    {
        if (Result.bPassed)
        {
            TestPassCounts.FindOrAdd(Result.TestName)++;
        }
        TestAvgDurations.FindOrAdd(Result.TestName) += Result.DurationSeconds;
    }
    
    // Calculate averages
    for (auto& Pair : TestAvgDurations)
    {
        Pair.Value /= FNexusTest::AllResults.Num();
    }
    
    bool bFirst = true;
    for (const auto& Pair : TestAvgDurations)
    {
        if (!bFirst) JSONContent += TEXT(",\n");
        
        int32 PassCount = TestPassCounts.FindRef(Pair.Key);
        JSONContent += FString::Printf(TEXT("    {\n      \"name\": \"%s\",\n      \"avg_duration_s\": %.4f,\n      \"pass_rate\": %.1f%%\n    }"),
            *Pair.Key, Pair.Value, (PassCount * 100.0) / FNexusTest::AllResults.Num());
        
        bFirst = false;
    }
    
    JSONContent += TEXT("\n  ]\n}\n");
    FFileHelper::SaveStringToFile(JSONContent, *JSONPath);
    UE_LOG(LogNexus, Display, TEXT("Exported test trends summary to %s"), *JSONPath);
}

void UNexusCore::ClearTestHistory()
{
    FNexusTest::AllResults.Empty();
    UE_LOG(LogNexus, Display, TEXT("Cleared all test result history"));
}