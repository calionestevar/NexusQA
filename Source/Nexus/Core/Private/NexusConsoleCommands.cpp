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

	UE_LOG(LogTemp, Warning, TEXT("🧪 NEXUS: Running %d test(s)..."), UNexusCore::TotalTests);
	UNexusCore::RunAllTests(true);  // true = parallel execution

	UE_LOG(LogTemp, Display, TEXT("✅ NEXUS: Complete — %d/%d passed"), 
		UNexusCore::PassedTests, UNexusCore::TotalTests);

	// Generate LCARS report
	TMap<FString, bool> Results;
	TMap<FString, double> Durations;
	TMap<FString, TArray<FString>> Artifacts;

	LCARSReporter::ExportResultsToLCARSFromPalantir(Results, Durations, Artifacts, TEXT("Saved/NexusReports/"));
	UE_LOG(LogTemp, Display, TEXT("📊 NEXUS: Report exported to Saved/NexusReports/"));
}
