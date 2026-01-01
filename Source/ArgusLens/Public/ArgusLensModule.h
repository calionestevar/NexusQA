#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * ArgusLens Performance Monitoring Module
 * 
 * Provides real-time performance monitoring and metrics collection
 * for Unreal Engine tests and applications
 */
class FArgusLensModule : public IModuleInterface
{
public:
	/**
	 * Called when the module is loaded into memory
	 * Initializes performance monitoring systems
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is getting unloaded
	 * Cleans up monitoring resources
	 */
	virtual void ShutdownModule() override;

	/**
	 * Check if the module has been successfully initialized
	 */
	static bool IsAvailable();
};
