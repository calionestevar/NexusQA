#pragma once
#include "CoreMinimal.h"

class LCARSReporter
{
public:
    static void ExportResultsToLCARS(const FAutomationTestFramework& Framework, const FString& OutputPath);
    // Export using Palantír in-memory maps (results, durations, artifacts)
    static void ExportResultsToLCARSFromPalantir(const TMap<FString, bool>& Results,
                                                 const TMap<FString, double>& Durations,
                                                 const TMap<FString, TArray<FString>>& Artifacts,
                                                 const FString& OutputPath);

    /**
     * Get the embedded HTML template for LCARS reports
     * Follows the same pattern as UObserverNetworkDashboard::GetEmbeddedHTMLTemplate()
     */
    static FString GetEmbeddedHTMLTemplate();
};