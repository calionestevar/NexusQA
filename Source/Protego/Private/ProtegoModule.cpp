#include "ProtegoModule.h"

#define LOCTEXT_NAMESPACE "FProtegoModule"

DEFINE_LOG_CATEGORY_STATIC(LogProtegoModule, Log, All);

static bool bProtegoModuleInitialized = false;

void FProtegoModule::StartupModule()
{
	UE_LOG(LogProtegoModule, Warning, TEXT("🛡️ PROTEGO COMPLIANCE MODULE INITIALIZING"));

	bProtegoModuleInitialized = true;

	UE_LOG(LogProtegoModule, Display, TEXT("✅ PROTEGO COMPLIANCE MODULE ONLINE"));
}

void FProtegoModule::ShutdownModule()
{
	UE_LOG(LogProtegoModule, Warning, TEXT("🛡️ PROTEGO COMPLIANCE MODULE SHUTTING DOWN"));

	bProtegoModuleInitialized = false;

	UE_LOG(LogProtegoModule, Display, TEXT("✅ PROTEGO COMPLIANCE MODULE SHUT DOWN"));
}

bool FProtegoModule::IsAvailable()
{
	return bProtegoModuleInitialized;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FProtegoModule, Protego);
