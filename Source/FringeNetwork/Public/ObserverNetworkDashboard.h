#pragma once
#include "CoreMinimal.h"
#include "ObserverNetworkDashboard.generated.h"

UCLASS()
class FRINGENETWORK_API UObserverNetworkDashboard : public UObject
{
    GENERATED_BODY()

public:
    // Call once at startup
    static void Initialize();

    // Call every frame when active
    static void UpdateLiveDashboard();

    // Call when a safety event occurs
    static void LogSafetyEvent(const FString& EventType, const FString& Details);

    // Generate final web report
    static void GenerateWebReport();
};