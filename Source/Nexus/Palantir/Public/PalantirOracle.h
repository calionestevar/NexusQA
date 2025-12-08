#pragma once
#include "CoreMinimal.h"

class PALANTIR_API FPalantirObserver
{
public:
    static void Initialize();                    // Called at startup
    static void UpdateLiveOverlay();             // Called every frame when active
    static void GenerateFinalReport();           // Called at test end
    static void OnTestStarted(const FString& Name);
    static void OnTestFinished(const FString& Name, bool bPassed);
    // Register an artifact (screenshot, log, replay) for a given test name.
    static void RegisterArtifact(const FString& TestName, const FString& ArtifactPath);
};