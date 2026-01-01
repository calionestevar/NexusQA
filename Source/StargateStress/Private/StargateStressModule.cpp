#include "StargateStressModule.h"

#define LOCTEXT_NAMESPACE "FStargateStressModule"

DEFINE_LOG_CATEGORY_STATIC(LogStargateStressModule, Log, All);

static bool bStargateStressModuleInitialized = false;

void FStargateStressModule::StartupModule()
{
	UE_LOG(LogStargateStressModule, Warning, TEXT("💥 STARGATE STRESS TESTING MODULE INITIALIZING"));

	bStargateStressModuleInitialized = true;

	UE_LOG(LogStargateStressModule, Display, TEXT("✅ STARGATE STRESS TESTING MODULE ONLINE"));
}

void FStargateStressModule::ShutdownModule()
{
	UE_LOG(LogStargateStressModule, Warning, TEXT("💥 STARGATE STRESS TESTING MODULE SHUTTING DOWN"));

	bStargateStressModuleInitialized = false;

	UE_LOG(LogStargateStressModule, Display, TEXT("✅ STARGATE STRESS TESTING MODULE SHUT DOWN"));
}

bool FStargateStressModule::IsAvailable()
{
	return bStargateStressModuleInitialized;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStargateStressModule, StargateStress);
