#include "NexusCore.h"
#include "NexusTest.h"
#include "Nexus/Palantir/Public/PalantirOracle.h"
#include "Misc/CommandLine.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include <atomic>

DEFINE_LOG_CATEGORY(LogNexus);

// Define the static test array from FNexusTest
// NEXUS_API on static member ensures proper DLL export for dependent modules
TArray<FNexusTest*> FNexusTest::AllTests;

int32 UNexusCore::TotalTests = 0;
int32 UNexusCore::PassedTests = 0;
int32 UNexusCore::FailedTests = 0;
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
    // Sort: Critical first, then Smoke, then Normal
    DiscoveredTests.Sort([](const FNexusTest& A, const FNexusTest& B) {
        return (int32)A.Priority > (int32)B.Priority;
    });

    if (!bParallel || DiscoveredTests.Num() <= 1)
    {
        RunSequentialWithFailFast();
        return;
    }

    // PARALLEL EXECUTION using Unreal's async task system
    UE_LOG(LogNexus, Display, TEXT("NEXUS: Running %d tests in parallel"), DiscoveredTests.Num());

    TArray<TFuture<bool>> Futures;
    std::atomic<bool> bCriticalFailed{false};

    for (FNexusTest* Test : DiscoveredTests)
    {
        if (!Test) continue;

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

            bool bPassed = Test->Execute();

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

    FPalantirObserver::GenerateFinalReport();
}

void UNexusCore::RegisterTest(FNexusTest* Test)
{
    if (Test)
    {
        DiscoveredTests.Add(Test);
    }
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
    for (FNexusTest* Test : DiscoveredTests)
    {
        if (!Test) continue;

        const FString Name = Test->TestName;
        FPalantirObserver::OnTestStarted(Name);
        NotifyTestStarted(Name);

        bool bPassed = Test->Execute();

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