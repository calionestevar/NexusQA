#pragma once

#include "CoreMinimal.h"

/**
 * Test result metadata captured by PalantirObserver
 */
struct FPalantirTestResult
{
	/** Whether the test passed */
	bool bPassed = false;

	/** Execution duration in seconds */
	double Duration = 0.0;

	/** Path to screenshot artifact if captured */
	FString ScreenshotPath;

	/** Path to trace file if exported */
	FString TraceFilePath;

	/** Path to log file if saved */
	FString LogFilePath;

	/** Optional error message */
	FString ErrorMessage;
};

/**
 * Report data structure for LCARS/HTML generation
 */
struct FReportData
{
	/** Report title */
	FString Title;

	/** List of test results */
	TMap<FString, FPalantirTestResult> TestResults;

	/** Total tests executed */
	int32 TotalTests = 0;

	/** Tests that passed */
	int32 PassedTests = 0;

	/** Tests that failed */
	int32 FailedTests = 0;

	/** Report generation timestamp */
	FDateTime GeneratedAt;
};
