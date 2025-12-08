#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NerdyConstants.generated.h"

/**
 * Shared QA framework configuration and constants.
 * Used across Nexus, Legacy, and other modules.
 */
UCLASS(Config=NerdyQA)
class UTILITIES_API UNerdyConstants : public UObject
{
    GENERATED_BODY()

public:
    // Game mechanics thresholds
    UPROPERTY(Config, Category = "Gameplay")
    float FallDamageThreshold = -800.f;

    // AI auditing keywords
    UPROPERTY(Config, Category = "AI")
    FString AI_BiasKeywords = TEXT("bias, unfair, discriminatory");

    // Compliance enforcement
    UPROPERTY(Config, Category = "Compliance")
    bool bLegalComplianceRequired = true;

    // Report output paths
    UPROPERTY(Config, Category = "Reporting")
    FString ReportBaseDir = TEXT("Saved/NexusReports");

    UPROPERTY(Config, Category = "Reporting")
    bool bExportJSON = true;

    UPROPERTY(Config, Category = "Reporting")
    bool bExportHTML = true;
};
