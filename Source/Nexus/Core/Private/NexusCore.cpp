#include "NexusCore.h"
#include "NexusTest.h"
#include "PalantirObserver.h"
#include "Misc/CommandLine.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include <atomic>

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
        UE_LOG(LogTemp, Display, TEXT("Legacy mode — delegating to Asgard"));
        return;
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
    // The magic: every NEXUS_TEST() creates a static global object
    // → its constructor adds itself to DiscoveredTests
    // → This runs automatically at startup
    UE_LOG(LogTemp, Display, TEXT("NEXUS: Auto-discovered %d test(s)"), DiscoveredTests.Num());
}

void UNexusCore::RunAllTests(bool bParallel)
{
    // Sort: Critical first, then Smoke, then Normal
    DiscoveredTests.Sort([](const FNexusTest* A, const FNexusTest* B) {
        return (int32)A->Priority > (int32)B->Priority;
    });

    if (!bParallel || DiscoveredTests.Num() <= 1)
    {
        RunSequentialWithFailFast();
        return;
    }

    // TRUE PARALLEL EXECUTION — 8 REALMS
    const int32 MaxWorkers = 8;
    const int32 TestsPerWorker = FMath::Max(1, DiscoveredTests.Num() / MaxWorkers);

    TArray<FProcHandle> WorkerHandles;
    WorkerHandles.SetNum(MaxWorkers);

    // Ensure no leftover abort sentinel exists from previous runs
    const FString AbortFile = GetAbortFilePath();
    if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*AbortFile))
    {
        FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*AbortFile);
    }

    for (int32 Worker = 0; Worker < MaxWorkers; ++Worker)
    {
        int32 Start = Worker * TestsPerWorker;
        int32 End = FMath::Min((Worker + 1) * TestsPerWorker, DiscoveredTests.Num());
        if (Start >= DiscoveredTests.Num()) break;

        FString CmdLine = FString::Printf(TEXT("-NexusWorker -start=%d -end=%d"), Start, End);

        FProcHandle Handle = FPlatformProcess::CreateProc(
            *FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/Win64/UnrealEditor-Cmd.exe"))),
            *FString::Printf(TEXT("\"%s\" %s -nullrhi -nosound -unattended -log -ExecCmds=\"Nexus.WorkerMode %s\""),
                *FPaths::ProjectFilePath(), *CmdLine, *CmdLine),
            true, false, false, nullptr, 0, nullptr, nullptr);

        WorkerHandles[Worker] = Handle;
    }

    // Wait for all workers with live monitoring
    bool bAnyCriticalFailed = false;
    while (true)
    {
        bool bAllDone = true;
        for (const FProcHandle& Handle : WorkerHandles)
        {
            if (FPlatformProcess::IsProcRunning(Handle))
            {
                bAllDone = false;
            }
            else
            {
                // If the process has finished, check its return code
                int32 ReturnCode = 0;
                if (FPlatformProcess::GetProcReturnCode(Handle, &ReturnCode))
                {
                    if (ReturnCode != 0)
                    {
                        UE_LOG(LogNexus, Error, TEXT("Worker exited with code %d"), ReturnCode);
                        bAnyCriticalFailed = true;
                        break;
                    }
                }
            }
        }

        // Check for early critical failure via sentinel file
        if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*AbortFile))
        {
            UE_LOG(LogNexus, Warning, TEXT("Detected abort sentinel file: %s"), *AbortFile);
            bAnyCriticalFailed = true;
            break;
        }

        if (bAllDone) break;
        // Periodic sleep used for small-poll heartbeats. This is a short
        // blocking wait (100ms) intended to throttle the loop and avoid
        // busy-waiting; if desired we can replace with event-based
        // synchronization in a follow-up to reduce latency and power usage.
        FPlatformProcess::Sleep(0.1f); // 100ms poll
    }

    // Kill any stragglers
    for (FProcHandle& Handle : WorkerHandles)
    {
        if (FPlatformProcess::IsProcRunning(Handle))
            FPlatformProcess::TerminateProc(Handle, true);
        FPlatformProcess::CloseProc(Handle);
    }

    if (bAnyCriticalFailed)
    {
        UE_LOG(LogNexus, Error, TEXT("CRITICAL FAILURE IN PARALLEL REALM — ABORTING ALL"));
        FPalantirObserver::GenerateFinalReport();
        return;
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