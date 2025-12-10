// LEGACY: AsgardCommandlet
//
// This demonstrates integration with Unreal's built-in automation framework.
// Shows competence with engine-native testing workflows while Nexus provides
// enhanced features (parallel execution, distributed tracing, fail-fast).
//
// Usage: UnrealEditor-Cmd.exe MyProject.uproject -run=Asgard

#include "AsgardCommandlet.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

int32 UAsgardCommandlet::Main(const FString& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("=== ASGARD COMMANDLET: Running UE Automation Tests ==="));

    // Get the automation controller
    IAutomationControllerModule& AutomationController = FModuleManager::LoadModuleChecked<IAutomationControllerModule>(TEXT("AutomationController"));
    IAutomationControllerManagerRef AutomationControllerManager = AutomationController.GetAutomationController();

    // Request available tests
    AutomationControllerManager->RequestAvailableWorkers(FGuid());
    AutomationControllerManager->RequestTests();

    // Simple synchronous wait for test discovery
    FPlatformProcess::Sleep(2.0f);

    // Run all available tests
    const bool bInRunningTests = AutomationControllerManager->RunTests();
    if (!bInRunningTests)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to start automation tests"));
        return 1;
    }

    // Wait for tests to complete (simple polling)
    while (AutomationControllerManager->GetTestState() == EAutomationControllerModuleState::Running)
    {
        FPlatformProcess::Sleep(0.5f);
    }

    // Export results to JSON
    const FString OutputPath = FPaths::ProjectSavedDir() / TEXT("Automation/AsgardReport.json");
    ExportTestResults(OutputPath, AutomationControllerManager);

    UE_LOG(LogTemp, Warning, TEXT("=== ASGARD COMPLETE: Report saved to %s ==="), *OutputPath);
    return 0;
}

void UAsgardCommandlet::ExportTestResults(const FString& OutputPath, IAutomationControllerManagerRef Controller)
{
    TSharedPtr<FJsonObject> JsonRoot = MakeShared<FJsonObject>();
    
    // Get test results
    const TArray<TSharedPtr<IAutomationReport>>& Reports = Controller->GetReports();
    TArray<TSharedPtr<FJsonValue>> TestArray;

    int32 Passed = 0;
    int32 Failed = 0;

    for (const TSharedPtr<IAutomationReport>& Report : Reports)
    {
        if (!Report.IsValid()) continue;

        TSharedPtr<FJsonObject> TestObj = MakeShared<FJsonObject>();
        TestObj->SetStringField(TEXT("Name"), Report->GetDisplayName());
        TestObj->SetBoolField(TEXT("Success"), Report->GetSuccessCount() > 0);
        TestObj->SetNumberField(TEXT("Duration"), Report->GetDuration());

        if (Report->GetSuccessCount() > 0)
            Passed++;
        else
            Failed++;

        TestArray.Add(MakeShared<FJsonValueObject>(TestObj));
    }

    JsonRoot->SetArrayField(TEXT("Tests"), TestArray);
    JsonRoot->SetNumberField(TEXT("TotalTests"), Reports.Num());
    JsonRoot->SetNumberField(TEXT("PassedTests"), Passed);
    JsonRoot->SetNumberField(TEXT("FailedTests"), Failed);

    // Write to file
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(JsonRoot.ToSharedRef(), Writer);
    FFileHelper::SaveStringToFile(JsonString, *OutputPath);
}