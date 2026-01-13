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
    static void RunTestsWithTags(ETestTag Tags, bool bParallel = true);  // Run only tests matching tags
    static void RegisterTest(class FNexusTest* Test);
    static bool EnsurePIEWorldActive();  // Auto-launch PIE if needed for game-thread tests
    static bool EnsurePIEWorldActive(const FString& MapPath);  // Auto-launch PIE if needed for game-thread tests
    static void RunSequentialWithFailFast();
    static FString GetAbortFilePath();
    static void SignalAbort(const FString& Reason = TEXT(""));

    // Reporting is now delegated
    static void NotifyTestStarted(const FString& Name);
    static void NotifyTestFinished(const FString& Name, bool bPassed);
    static void NotifyTestSkipped(const FString& Name);
    
    // Test filtering
    static TArray<class FNexusTest*> GetTestsWithTags(ETestTag Tags);  // Get all tests matching tags
    static int32 CountTestsWithTags(ETestTag Tags);  // Count tests matching tags
    static TArray<class FNexusTest*> GetTestsWithCustomTag(const FString& CustomTag);  // Get all tests with custom tag
    static int32 CountTestsWithCustomTag(const FString& CustomTag);  // Count tests with custom tag
    static TArray<FString> GetAllCustomTags();  // Get all unique custom tags across tests
    
    // Test result history & trend analysis
    static double GetAverageTestDuration(const FString& TestName = TEXT(""));  // Average duration for all tests or specific test
    static double GetMedianTestDuration(const FString& TestName = TEXT(""));   // Median duration for trend detection
    static int32 DetectRegressions(double MaxAllowedDurationMs = 0.0);         // Detect tests slower than baseline
    static void ExportTestTrends(const FString& OutputPath = TEXT(""));        // Export trend report to JSON/CSV
    static void ClearTestHistory();                                             // Clear all stored test results

    // Stats
    static int32 TotalTests;
    static int32 PassedTests;
    static int32 FailedTests;
    static int32 SkippedTests;
    static int32 CriticalTests;
    static TArray<class FNexusTest*> DiscoveredTests;
private:
    FString GetConfiguredTestMap() const;
};