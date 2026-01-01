#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Utilities Module
 * 
 * Provides common utility functions, helpers, and shared infrastructure
 * for other NexusQA modules
 */
class FUtilitiesModule : public IModuleInterface
{
public:
	/**
	 * Called when the module is loaded into memory
	 * Initializes utility systems
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is getting unloaded
	 * Cleans up utility resources
	 */
	virtual void ShutdownModule() override;

	/**
	 * Check if the module has been successfully initialized
	 */
	static bool IsAvailable();
};
