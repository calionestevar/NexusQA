#pragma once
#include "CoreMinimal.h"
#include "PalantirTypes.h"

/**
 * Test Result Oracle - Central repository for test execution results
 * 
 * Manages in-memory storage of test results, artifacts, and metadata.
 * Acts as the single source of truth for test execution data during and after test runs.
 */
class NEXUS_API FPalantirOracle
{
public:
	/** Get the singleton instance */
	static FPalantirOracle& Get();

	/** Register a new test result */
	void RecordTestResult(const FString& TestName, const FPalantirTestResult& Result);

	/** Get all test results */
	const TMap<FString, FPalantirTestResult>& GetAllTestResults() const;

	/** Get a specific test result by name */
	const FPalantirTestResult* GetTestResult(const FString& TestName) const;

	/** Clear all recorded results */
	void ClearAllResults();

	/** Get total test count */
	int32 GetTotalTestCount() const;

	/** Get passed test count */
	int32 GetPassedTestCount() const;

	/** Get failed test count */
	int32 GetFailedTestCount() const;

private:
	FPalantirOracle() = default;

	/** In-memory storage of test results */
	TMap<FString, FPalantirTestResult> TestResults;

	/** Synchronization for thread-safe access */
	mutable FCriticalSection ResultsLock;
};

class NEXUS_API FPalantirObserver
{
public:
    static void Initialize();                    // Called at startup
    static void UpdateLiveOverlay();             // Called every frame when active
    static void GenerateFinalReport();           // Called at test end
    static void OnTestStarted(const FString& Name);
    static void OnTestFinished(const FString& Name, bool bPassed);
    // Register an artifact (screenshot, log, replay) for a given test name.
    static void RegisterArtifact(const FString& TestName, const FString& ArtifactPath);
};