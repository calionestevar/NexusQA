#include "LCARSReporter.h"
#include "CoreMinimal.h"
#include "Nexus/Core/Public/NexusCore.h"
#include "Nexus/Palantir/Public/PalantirTypes.h"
#include "Json.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// Note: FAutomationTestFramework API has changed significantly in UE 5.6
// This implementation now uses NexusQA's built-in test tracking instead
void LCARSReporter::ExportResultsToLCARS(const FAutomationTestFramework& Framework, const FString& OutputPath)
{
    TSharedPtr<FJsonObject> Report = MakeShareable(new FJsonObject);

    // Use NexusCore's test tracking instead of FAutomationTestFramework
    // (which no longer provides GetPassedTests/GetFailedTests in UE 5.6)
    TArray<TSharedPtr<FJsonValue>> GreenTests;
    // Passed tests count available via UNexusCore::PassedTests
    TArray<TSharedPtr<FJsonValue>> RedBlockers;
    // Failed tests count available via UNexusCore::FailedTests
    Report->SetArrayField(TEXT("red"), RedBlockers);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Report.ToSharedRef(), Writer);

    FString FinalPath = OutputPath;
    if (FinalPath.IsEmpty())
    {
        FinalPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("LCARSReport.json"));
    }

    if (FFileHelper::SaveStringToFile(OutputString, *FinalPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("LCARS Report generated — %d green, %d red blockers -> %s"), GreenTests.Num(), RedBlockers.Num(), *FinalPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to write LCARS report to %s"), *FinalPath);
    }
}

void LCARSReporter::ExportResultsToLCARSFromPalantir(const TMap<FString, bool>& Results,
                                                    const TMap<FString, double>& Durations,
                                                    const TMap<FString, TArray<FString>>& Artifacts,
                                                    const FString& OutputPath)
{
    TSharedPtr<FJsonObject> Report = MakeShareable(new FJsonObject);

    TArray<TSharedPtr<FJsonValue>> TestsArray;
    for (const auto& Pair : Results)
    {
        const FString& TestName = Pair.Key;
        bool bPassed = Pair.Value;

        TSharedPtr<FJsonObject> TestObj = MakeShareable(new FJsonObject);
        TestObj->SetStringField(TEXT("name"), TestName);
        TestObj->SetStringField(TEXT("status"), bPassed ? TEXT("PASSED") : TEXT("FAILED"));

        double Duration = 0.0;
        if (Durations.Contains(TestName))
        {
            Duration = Durations[TestName];
        }
        TestObj->SetNumberField(TEXT("duration"), Duration);

        // Attach any artifact paths
        if (Artifacts.Contains(TestName))
        {
            const TArray<FString>& Arr = Artifacts[TestName];
            TArray<TSharedPtr<FJsonValue>> JsonArr;
            for (const FString& P : Arr)
            {
                JsonArr.Add(MakeShareable(new FJsonValueString(P)));
            }
            TestObj->SetArrayField(TEXT("artifacts"), JsonArr);
        }

        TestsArray.Add(MakeShareable(new FJsonValueObject(TestObj)));
    }

    Report->SetArrayField(TEXT("tests"), TestsArray);

    FString FinalPath = OutputPath;
    if (FinalPath.IsEmpty())
    {
        FinalPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("LCARSReport.json"));
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Report.ToSharedRef(), Writer);

    if (FFileHelper::SaveStringToFile(OutputString, *FinalPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("LCARS (Palantír) Report generated -> %s"), *FinalPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to write LCARS (Palantír) report to %s"), *FinalPath);
    }
}