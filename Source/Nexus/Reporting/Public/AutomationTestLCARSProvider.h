#pragma once

#include "CoreMinimal.h"
#include "LCARSProvider.h"

/**
 * LCARS Provider implementation that reads from Unreal Engine's AutomationTestFramework
 * 
 * This provider integrates with UE5's native automation system, allowing LCARS reports
 * to be generated from official Unreal automation tests.
 */
class NEXUS_API FAutomationTestLCARSProvider : public ILCARSResultsProvider
{
public:
	FAutomationTestLCARSProvider();
	virtual ~FAutomationTestLCARSProvider() = default;

	virtual FLCARSResults GetResults() override;

private:
	/**
	 * Queries FAutomationTestFramework for all test results
	 * and transforms them into LCARS format
	 */
	void PopulateFromAutomationFramework();

	/** Cached results from last framework query */
	FLCARSResults CachedResults;
	
	/** Whether cache needs refresh */
	bool bNeedsRefresh;
};
