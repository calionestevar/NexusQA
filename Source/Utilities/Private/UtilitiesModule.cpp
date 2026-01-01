#include "UtilitiesModule.h"

#define LOCTEXT_NAMESPACE "FUtilitiesModule"

DEFINE_LOG_CATEGORY_STATIC(LogUtilitiesModule, Log, All);

static bool bUtilitiesModuleInitialized = false;

void FUtilitiesModule::StartupModule()
{
	UE_LOG(LogUtilitiesModule, Warning, TEXT("🔧 UTILITIES MODULE INITIALIZING"));

	bUtilitiesModuleInitialized = true;

	UE_LOG(LogUtilitiesModule, Display, TEXT("✅ UTILITIES MODULE ONLINE"));
}

void FUtilitiesModule::ShutdownModule()
{
	UE_LOG(LogUtilitiesModule, Warning, TEXT("🔧 UTILITIES MODULE SHUTTING DOWN"));

	bUtilitiesModuleInitialized = false;

	UE_LOG(LogUtilitiesModule, Display, TEXT("✅ UTILITIES MODULE SHUT DOWN"));
}

bool FUtilitiesModule::IsAvailable()
{
	return bUtilitiesModuleInitialized;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUtilitiesModule, Utilities);
