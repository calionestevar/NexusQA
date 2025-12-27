#pragma once

#include "CoreMinimal.h"
#include "LCARSProvider.h"

// Forward declare FPalantirOracle (defined in Palantir module)
class FPalantirOracle;

/**
 * LCARS Provider implementation that reads from PalantirOracle's in-memory test results
 * 
 * This is the primary provider for the Nexus framework, pulling directly from
 * the Palantír subsystem's test execution records.
 */
class NEXUS_API FPalantirLCARSProvider : public ILCARSResultsProvider
{
public:
	FPalantirLCARSProvider(FPalantirOracle* InOracle);
	virtual ~FPalantirLCARSProvider() = default;

	virtual FLCARSResults GetResults() override;

private:
	FPalantirOracle* Oracle;
};
