#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * FringeNetwork Module
 * 
 * Provides network testing, observer pattern coordination, and
 * parallel realm testing capabilities for distributed testing scenarios
 */
class FFringeNetworkModule : public IModuleInterface
{
public:
	/**
	 * Called when the module is loaded into memory
	 * Initializes network testing systems
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is getting unloaded
	 * Cleans up network resources
	 */
	virtual void ShutdownModule() override;

	/**
	 * Check if the module has been successfully initialized
	 */
	static bool IsAvailable();
};
