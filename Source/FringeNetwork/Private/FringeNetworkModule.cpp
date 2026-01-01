#include "FringeNetworkModule.h"

#define LOCTEXT_NAMESPACE "FFringeNetworkModule"

DEFINE_LOG_CATEGORY_STATIC(LogFringeNetworkModule, Log, All);

static bool bFringeNetworkModuleInitialized = false;

void FFringeNetworkModule::StartupModule()
{
	UE_LOG(LogFringeNetworkModule, Warning, TEXT("🌐 FRINGE NETWORK MODULE INITIALIZING"));

	bFringeNetworkModuleInitialized = true;

	UE_LOG(LogFringeNetworkModule, Display, TEXT("✅ FRINGE NETWORK MODULE ONLINE"));
}

void FFringeNetworkModule::ShutdownModule()
{
	UE_LOG(LogFringeNetworkModule, Warning, TEXT("🌐 FRINGE NETWORK MODULE SHUTTING DOWN"));

	bFringeNetworkModuleInitialized = false;

	UE_LOG(LogFringeNetworkModule, Display, TEXT("✅ FRINGE NETWORK MODULE SHUT DOWN"));
}

bool FFringeNetworkModule::IsAvailable()
{
	return bFringeNetworkModuleInitialized;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFringeNetworkModule, FringeNetwork);
