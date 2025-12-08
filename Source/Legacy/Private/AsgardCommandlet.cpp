// LEGACY: AsgardCommandlet -- legacy commandlet entrypoint
//
// This commandlet demonstrates use of Unreal's Application Commandlet
// pattern to run the engine's automation tests and export a JSON report.
// It is preserved for compatibility and to show familiarity with engine-native
// testing workflows. For CI and large-scale parallel runs prefer `Nexus`.

#include "CoreMinimal.h"
#include "Misc/CoreDelegates.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "AutomationTestRunner.h"

IMPLEMENT_APPLICATION_COMMANDLET(FAsgardCommandlet, Asgard, TEXT("Runs full QA suite and exports JSON report"));

FAsgardCommandlet::FAsgardCommandlet()
{
    IsClientOnly = true;
    IsEditor = false;
    IsServer = false;
}

int32 FAsgardCommandlet::Main(const FString& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("*** ASGARD COMMANDLET ACTIVATED — BEAMING TESTS ***"));

    // Run all tests
    FAutomationTestRunner::RunAllTests();

    // Export JSON report
    FAutomationTestFramework::GetInstance().ExportResultsToJSON(TEXT("LCARSReport.json"));

    UE_LOG(LogTemp, Warning, TEXT("*** ASGARD COMPLETE — REPORT EXPORTED TO LCARSReport.json ***"));
    return 0;
}