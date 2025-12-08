#pragma once
#include "CoreMinimal.h"
#include "NerdyConstants.generated.h"

UCLASS(Config=NerdyQA)
class UNerdyConstants : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, Category = "AsgardCore")
    float FallDamageThreshold = -800.f;

    UPROPERTY(Config, Category = "PalantirAnalyzer")
    FString AI_BiasKeywords = TEXT("bias, unfair, discriminatory");

    UPROPERTY(Config, Category = "GoauldGatekeeper")
    bool LegalComplianceRequired = true;
};