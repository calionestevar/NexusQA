#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NexusTest.h"
#include "NexusCore.generated.h"

UCLASS()
class NEXUS_API UNexusCore : public UObject
{
    GENERATED_BODY()

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

    // Stats
    static int32 TotalTests;
    static int32 PassedTests;
    static int32 FailedTests;
    static int32 CriticalTests;
    static TArray<class FNexusTest*> DiscoveredTests;
};