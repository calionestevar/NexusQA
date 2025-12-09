#pragma once
#include "CoreMinimal.h"

// Lightweight container for LCARS results used by providers
struct FLCARSResults
{
    TMap<FString, bool> Results;
    TMap<FString, double> Durations;
    TMap<FString, TArray<FString>> Artifacts;
};

// Interface for pluggable LCARS results providers
class ILCARSResultsProvider
{
public:
    virtual ~ILCARSResultsProvider() {}
    virtual FLCARSResults GetResults() = 0;
};
