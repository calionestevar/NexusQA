#pragma once
#include "CoreMinimal.h"
#include "DefenseAgainstTheDarkArts.generated.h"

UENUM(BlueprintType)
enum class EComplianceStandard : uint8
{
    COPPA,
    GDPR,
    DSA,
    Custom
};

/**
 * DefenseAgainstTheDarkArts — Minor protection and compliance audits (COPPA/GDPR/DSA)
 */
UCLASS()
class UDefenseAgainstTheDarkArts : public UObject
{
    GENERATED_BODY()

public:
    // Run full compliance audit
    UFUNCTION(BlueprintCallable, Category = "Protego|Compliance")
    static void PerformComplianceAudit(EComplianceStandard Standard = EComplianceStandard::COPPA);

    // Individual checks
    UFUNCTION(BlueprintCallable, Category = "Protego|Compliance|COPPA")
    static bool VerifyAgeGatePreventsVoiceChat();

    UFUNCTION(BlueprintCallable, Category = "Protego|Compliance|GDPR")
    static bool VerifyNoPersonalDataStoredWithoutConsent();

    UFUNCTION(BlueprintCallable, Category = "Protego|Compliance|DSA")
    static bool VerifyReportSystemEscalatesWithin24h();
};
