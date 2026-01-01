#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Protego Compliance Module
 * 
 * Provides compliance checking, security validation, and regulatory
 * requirement enforcement for Unreal Engine applications
 */
class FProtegoModule : public IModuleInterface
{
public:
	/**
	 * Called when the module is loaded into memory
	 * Initializes compliance checking systems
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is getting unloaded
	 * Cleans up compliance resources
	 */
	virtual void ShutdownModule() override;

	/**
	 * Check if the module has been successfully initialized
	 */
	static bool IsAvailable();
};
