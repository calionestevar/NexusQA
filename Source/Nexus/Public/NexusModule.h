#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

/**
 * NexusQA Test Framework Module
 * 
 * Provides test orchestration, API request tracing, performance monitoring,
 * and compliance checking for Unreal Engine 5.7+
 */
class FNexusModule : public IModuleInterface
{
public:
	/**
	 * Called when the module is loaded into memory
	 * Initializes test discovery and framework systems
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is getting unloaded
	 * Cleans up test data and framework resources
	 */
	virtual void ShutdownModule() override;

	/**
	 * Check if the module has been successfully initialized
	 */
	static bool IsAvailable();
};

// Declare log category for use throughout the module
DECLARE_LOG_CATEGORY_EXTERN(LogNexusModule, Log, All);
