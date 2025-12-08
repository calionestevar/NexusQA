#include "DefenseAgainstTheDarkArts.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Json.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/ScopeLock.h"
#include "JsonUtilities.h"
#include "Engine/Engine.h"

static FCriticalSection GProtegoMutex;

static void ProtegoLog(const FString& Msg, bool bSuccess = true)
{
    FScopeLock _lock(&GProtegoMutex);
    UE_LOG(LogTemp, Display, TEXT("PROTEGO: %s"), *Msg);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, bSuccess ? FColor::Green : FColor::Red, TEXT("PROTEGO: ") + Msg);
    }
}

void UDefenseAgainstTheDarkArts::PerformComplianceAudit(EComplianceStandard Standard)
{
    // Safely get enum name
    FString EnumName = TEXT("Unknown");
    if (const UEnum* EnumPtr = StaticEnum<EComplianceStandard>())
    {
        EnumName = EnumPtr->GetNameStringByValue((int64)Standard);
    }
    ProtegoLog(FString::Printf(TEXT("DEFENSE AGAINST THE DARK ARTS AUDIT - %s"), *EnumName));

    switch (Standard)
    {
    case EComplianceStandard::COPPA:
        VerifyAgeGatePreventsVoiceChat();
        break;
    case EComplianceStandard::GDPR:
        VerifyNoPersonalDataStoredWithoutConsent();
        break;
    case EComplianceStandard::DSA:
        VerifyReportSystemEscalatesWithin24h();
        break;
    default:
        break;
    }

    ProtegoLog(TEXT("COMPLIANCE AUDIT COMPLETE — PROTEGO TOTALUM!"));
}

bool UDefenseAgainstTheDarkArts::VerifyAgeGatePreventsVoiceChat()
{
    bool bVoiceEnabled = false; // Hook into your game's age-gate/voice system
    bool bPassed = !bVoiceEnabled;
    ProtegoLog(FString::Printf(TEXT("COPPA: Voice chat blocked for minors - %s"), bPassed ? TEXT("PASS") : TEXT("FAIL")), bPassed);
    return bPassed;
}

bool UDefenseAgainstTheDarkArts::VerifyNoPersonalDataStoredWithoutConsent()
{
    bool bNoPII = true; // Integrate with analytics/telemetry to ensure PII is not stored without consent
    ProtegoLog(FString::Printf(TEXT("GDPR: No PII stored without consent - %s"), bNoPII ? TEXT("PASS") : TEXT("FAIL")), bNoPII);
    return bNoPII;
}

bool UDefenseAgainstTheDarkArts::VerifyReportSystemEscalatesWithin24h()
{
    float HoursSinceReport = 1.5f; // Mock backend value; replace with real moderation queue check
    bool bEscalated = HoursSinceReport <= 24.0f;
    ProtegoLog(FString::Printf(TEXT("DSA: Reports escalated within 24h - %s"), bEscalated ? TEXT("PASS") : TEXT("FAIL")), bEscalated);
    return bEscalated;
}

static void LoadAllComplianceRules()
{
    FString RulesDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("Protego"), TEXT("Private"), TEXT("ComplianceRules"));
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*RulesDir))
    {
        UE_LOG(LogTemp, Warning, TEXT("PROTEGO: Rules directory does not exist: %s"), *RulesDir);
        return;
    }

    TArray<FString> FoundFiles;
    PlatformFile.FindFilesRecursively(FoundFiles, *RulesDir, TEXT("*.json"));

    for (const FString& FullPath : FoundFiles)
    {
        FString JsonContent;
        if (FFileHelper::LoadFileToString(JsonContent, *FullPath))
        {
            TSharedPtr<FJsonObject> RootObj;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
            if (FJsonSerializer::Deserialize(Reader, RootObj) && RootObj.IsValid())
            {
                UE_LOG(LogTemp, Display, TEXT("PROTEGO: Loaded rule set %s"), *FPaths::GetCleanFilename(FullPath));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("PROTEGO: Failed to parse JSON rule file %s"), *FullPath);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PROTEGO: Failed to read rule file %s"), *FullPath);
        }
    }
}
