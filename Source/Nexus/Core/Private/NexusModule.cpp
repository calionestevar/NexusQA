#include "CoreMinimal.h"
#include "NexusModule.h"
#include "Nexus/Core/Public/NexusCore.h"
#include "Nexus/Palantir/Public/PalantirOracle.h"

#define LOCTEXT_NAMESPACE "FNexusModule"

DEFINE_LOG_CATEGORY(LogNexusModule);

static bool bNexusModuleInitialized = false;

void FNexusModule::StartupModule()
{
	UE_LOG(LogNexusModule, Warning, TEXT("🧪 NEXUS TEST FRAMEWORK INITIALIZING — UE 5.7"));

	// Discover all NEXUS_TEST macros in loaded modules
	UNexusCore::DiscoverAllTests();

	// Initialize PalantirOracle for test result tracking
	FPalantirOracle::Get();

	bNexusModuleInitialized = true;

	UE_LOG(LogNexusModule, Display, TEXT("✅ NEXUS FRAMEWORK ONLINE — %d tests discovered"), UNexusCore::TotalTests);
}

void FNexusModule::ShutdownModule()
{
	UE_LOG(LogNexusModule, Warning, TEXT("🧪 NEXUS TEST FRAMEWORK SHUTTING DOWN"));

	// Clean up test data
	UNexusCore::TotalTests = 0;
	UNexusCore::PassedTests = 0;
	UNexusCore::FailedTests = 0;
	UNexusCore::CriticalTests = 0;
	UNexusCore::DiscoveredTests.Empty();

	// Clear PalantirOracle results
	FPalantirOracle::Get().ClearAllResults();

	bNexusModuleInitialized = false;

	UE_LOG(LogNexusModule, Display, TEXT("✅ NEXUS FRAMEWORK SHUT DOWN"));
}

bool FNexusModule::IsAvailable()
{
	return bNexusModuleInitialized;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNexusModule, Nexus);
