#include "PalantirLCARSProvider.h"
#include "Nexus/Palantir/Public/PalantirOracle.h"

FPalantirLCARSProvider::FPalantirLCARSProvider(FPalantirOracle* InOracle)
	: Oracle(InOracle)
{
	check(Oracle != nullptr);
}

FLCARSResults FPalantirLCARSProvider::GetResults()
{
	FLCARSResults Results;

	if (!Oracle)
	{
		return Results;
	}

	// Get all test results from PalantirOracle
	const TMap<FString, FPalantirTestResult>& TestResults = Oracle->GetAllTestResults();

	for (const auto& Pair : TestResults)
	{
		const FString& TestName = Pair.Key;
		const FPalantirTestResult& TestResult = Pair.Value;

		// Add pass/fail result
		Results.Results.Add(TestName, TestResult.bPassed);

		// Add duration
		Results.Durations.Add(TestName, TestResult.Duration);

		// Add artifacts (screenshots, traces, logs)
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
			Results.Artifacts.Add(TestName, Artifacts);
		}
	}

	return Results;
}
