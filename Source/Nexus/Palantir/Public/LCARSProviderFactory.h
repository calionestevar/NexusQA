#pragma once

#include "CoreMinimal.h"
#include "LCARSProvider.h"

/**
 * Factory for creating and managing LCARS result providers
 * 
 * Usage:
 *   TUniquePtr<ILCARSResultsProvider> Provider = FLCARSProviderFactory::CreateProvider(ELCARSProviderType::Palantir);
 *   FLCARSResults Results = Provider->GetResults();
 */
enum class ELCARSProviderType
{
	Palantir,         // Read from PalantirOracle in-memory results
	AutomationTest,   // Read from UE5 AutomationTestFramework
	Custom            // User-provided custom implementation
};

class NEXUS_API FLCARSProviderFactory
{
public:
	/**
	 * Create a provider instance by type
	 */
	static TUniquePtr<ILCARSResultsProvider> CreateProvider(ELCARSProviderType Type, void* InContext = nullptr);

	/**
	 * Register a custom provider type
	 */
	static void RegisterCustomProvider(const FString& Name, TFunction<TUniquePtr<ILCARSResultsProvider>()> Factory);

	/**
	 * Get provider by registered name
	 */
	static TUniquePtr<ILCARSResultsProvider> CreateCustomProvider(const FString& Name);
};
