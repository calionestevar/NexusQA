#include "ArgusLensModule.h"
#include "ArgusLens.h"

#define LOCTEXT_NAMESPACE "FArgusLensModule"

DEFINE_LOG_CATEGORY_STATIC(LogArgusLensModule, Log, All);

static bool bArgusLensModuleInitialized = false;

void FArgusLensModule::StartupModule()
{
	UE_LOG(LogArgusLensModule, Warning, TEXT("📊 ARGUSLENS PERFORMANCE MONITOR INITIALIZING"));

	// Initialize ArgusLens performance monitoring
	UArgusLens::Initialize();

	bArgusLensModuleInitialized = true;

	UE_LOG(LogArgusLensModule, Display, TEXT("✅ ARGUSLENS PERFORMANCE MONITOR ONLINE"));
}

void FArgusLensModule::ShutdownModule()
{
	UE_LOG(LogArgusLensModule, Warning, TEXT("📊 ARGUSLENS PERFORMANCE MONITOR SHUTTING DOWN"));

	// Cleanup
	bArgusLensModuleInitialized = false;

	UE_LOG(LogArgusLensModule, Display, TEXT("✅ ARGUSLENS PERFORMANCE MONITOR SHUT DOWN"));
}

bool FArgusLensModule::IsAvailable()
{
	return bArgusLensModuleInitialized;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FArgusLensModule, ArgusLens);
