#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * StargateStress Load Testing Module
 * 
 * Provides stress testing, load generation, and system resilience
 * validation for Unreal Engine applications
 */
class FStargateStressModule : public IModuleInterface
{
public:
	/**
	 * Called when the module is loaded into memory
	 * Initializes stress testing systems
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is getting unloaded
	 * Cleans up stress testing resources
	 */
	virtual void ShutdownModule() override;

	/**
	 * Check if the module has been successfully initialized
	 */
	static bool IsAvailable();
};
