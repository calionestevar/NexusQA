#include "NexusConsoleCommands.h"
#include "NexusCore.h"
#include "Nexus/LCARSBridge/Public/LCARSReporter.h"
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

	UE_LOG(LogTemp, Warning, TEXT("🧪 NEXUS: Running %d test(s)..."), UNexusCore::TotalTests);
	UNexusCore::RunAllTests(true);  // true = parallel execution

	UE_LOG(LogTemp, Display, TEXT("✅ NEXUS: Complete — %d/%d passed"), 
		UNexusCore::PassedTests, UNexusCore::TotalTests);

	// Generate LCARS report
	TMap<FString, bool> Results;
	TMap<FString, double> Durations;
	TMap<FString, TArray<FString>> Artifacts;

	const FString LcarsPath = FPaths::ProjectSavedDir() / TEXT("NexusReports");
	LCARSReporter::ExportResultsToLCARSFromPalantir(Results, Durations, Artifacts, LcarsPath);
	UE_LOG(LogTemp, Display, TEXT("📊 NEXUS: Report exported to %s"), *LcarsPath);
}
