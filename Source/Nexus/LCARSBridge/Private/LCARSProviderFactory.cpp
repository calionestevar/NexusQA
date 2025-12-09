#include "LCARSProviderFactory.h"
#include "PalantirLCARSProvider.h"
#include "AutomationTestLCARSProvider.h"

// Static map for custom providers
static TMap<FString, TFunction<TUniquePtr<ILCARSResultsProvider>()>>& GetCustomProviders()
{
	static TMap<FString, TFunction<TUniquePtr<ILCARSResultsProvider>()>> Providers;
	return Providers;
}

TUniquePtr<ILCARSResultsProvider> FLCARSProviderFactory::CreateProvider(ELCARSProviderType Type, void* InContext)
{
	switch (Type)
	{
		case ELCARSProviderType::Palantir:
		{
			FPalantirOracle* Oracle = static_cast<FPalantirOracle*>(InContext);
			if (!Oracle)
			{
				UE_LOG(LogTemp, Warning, TEXT("FLCARSProviderFactory: Palantir provider requires FPalantirOracle context"));
				return nullptr;
			}
			return MakeUnique<FPalantirLCARSProvider>(Oracle);
		}

		case ELCARSProviderType::AutomationTest:
		{
			return MakeUnique<FAutomationTestLCARSProvider>();
		}

		case ELCARSProviderType::Custom:
		{
			UE_LOG(LogTemp, Warning, TEXT("FLCARSProviderFactory: Custom provider type requires named lookup"));
			return nullptr;
		}

		default:
		{
			UE_LOG(LogTemp, Error, TEXT("FLCARSProviderFactory: Unknown provider type"));
			return nullptr;
		}
	}
}

void FLCARSProviderFactory::RegisterCustomProvider(
	const FString& Name,
	TFunction<TUniquePtr<ILCARSResultsProvider>()> Factory)
{
	GetCustomProviders().Add(Name, Factory);
	UE_LOG(LogTemp, Display, TEXT("FLCARSProviderFactory: Registered custom provider '%s'"), *Name);
}

TUniquePtr<ILCARSResultsProvider> FLCARSProviderFactory::CreateCustomProvider(const FString& Name)
{
	const auto& Providers = GetCustomProviders();
	if (Providers.Contains(Name))
	{
		return Providers[Name]();
	}

	UE_LOG(LogTemp, Warning, TEXT("FLCARSProviderFactory: Custom provider '%s' not found"), *Name);
	return nullptr;
}
