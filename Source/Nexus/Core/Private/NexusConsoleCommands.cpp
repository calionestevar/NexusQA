#include "NexusConsoleCommands.h"
#include "NexusCore.h"
#include "Nexus/Core/Public/NexusTest.h"
#include "Nexus/LCARSBridge/Public/LCARSReporter.h"
#include "Nexus/Palantir/Public/PalantirOracle.h"
#include "HAL/IConsoleManager.h"

void FNexusConsoleCommands::Register()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Nexus.RunTests"),
		TEXT("Execute all discovered NEXUS_TEST macros and generate LCARS report"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&FNexusConsoleCommands::OnRunTests)
	);
}

void FNexusConsoleCommands::OnRunTests(const TArray<FString>& Args)
{
	UE_LOG(LogTemp, Warning, TEXT("🧪 NEXUS: Discovering tests..."));
	UNexusCore::DiscoverAllTests();

	// Attempt to ensure PIE world is active for game-thread tests
	if (!UNexusCore::EnsurePIEWorldActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  No active game world detected — Game-thread tests will gracefully skip"));
		UE_LOG(LogTemp, Display, TEXT("💡 To run game-thread tests with full world context, click 'Play' in the editor first"));
	}

	int32 TotalTests = FNexusTest::AllTests.Num();
	UE_LOG(LogTemp, Warning, TEXT("🧪 NEXUS: Running %d test(s)..."), TotalTests);
	UNexusCore::RunAllTests(true);  // true = parallel execution

	// Calculate pass/fail/skip counts from results
	int32 PassedCount = 0;
	int32 FailedCount = 0;
	int32 SkippedCount = 0;
	
	for (const FNexusTestResult& Result : FNexusTest::AllResults)
	{
		if (Result.bSkipped)
		{
			SkippedCount++;
		}
		else if (Result.bPassed)
		{
			PassedCount++;
		}
		else
		{
			FailedCount++;
		}
	}

	UE_LOG(LogTemp, Display, TEXT("✅ NEXUS: Complete — %d/%d passed"), PassedCount, TotalTests);

	// Populate maps with actual test results from AllResults
	TMap<FString, bool> Results;
	TMap<FString, double> Durations;
	TMap<FString, TArray<FString>> Artifacts;

	for (const FNexusTestResult& Result : FNexusTest::AllResults)
	{
		Results.Add(Result.TestName, Result.bPassed);
		Durations.Add(Result.TestName, Result.DurationSeconds);
		
		// Artifacts would be added by the observer/reporter system
		// For now, we skip artifact collection from results
	}

	const FString LcarsPath = FPaths::ProjectSavedDir() / TEXT("NexusReports");
	LCARSReporter::ExportResultsToLCARSFromPalantir(Results, Durations, Artifacts, LcarsPath);
	UE_LOG(LogTemp, Display, TEXT("📊 NEXUS: Report exported to %s"), *LcarsPath);
}
