#pragma once
#include "CoreMinimal.h"
#include "NexusTest.h"

/**
 * NexusCore - Test framework coordinator
 * Pure static class for test orchestration and lifecycle management
 * Note: No UCLASS() - this is a utility class, not a UObject, to avoid DLL reflection issues
 */
class NEXUS_API UNexusCore
{
public:
    static void Execute(const TArray<FString>& Args);
    static void DiscoverAllTests();
    static void RunAllTests(bool bParallel = true);
    static void RegisterTest(class FNexusTest* Test);
    static void RunSequentialWithFailFast();
    static FString GetAbortFilePath();
    static void SignalAbort(const FString& Reason = TEXT(""));

    // Reporting is now delegated
    static void NotifyTestStarted(const FString& Name);
    static void NotifyTestFinished(const FString& Name, bool bPassed);
    static void NotifyTestSkipped(const FString& Name);

    // Stats
    static int32 TotalTests;
    static int32 PassedTests;
    static int32 FailedTests;
    static int32 SkippedTests;
    static int32 CriticalTests;
    static TArray<class FNexusTest*> DiscoveredTests;
};