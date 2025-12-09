#pragma once
#include "CoreMinimal.h"
#include "FringeNetwork.generated.h"

UCLASS()
class FRINGENETWORK_API UFringeNetwork : public UObject
{
    GENERATED_BODY()

public:
    // Master command — runs the full Observer Network
    UFUNCTION(BlueprintCallable, Category = "Fringe Network")
    static void ActivateObserverNetwork();

    // Individual Observers
    UFUNCTION(BlueprintCallable, Category = "Fringe Network|Observers")
    static void RunObserverNetworkTests(const FString& PrimaryServer);

    UFUNCTION(BlueprintCallable, Category = "Fringe Network|Parallel Realms")
    static void TestParallelRealms(const TArray<FString>& RegionURLs);

    UFUNCTION(BlueprintCallable, Category = "Fringe Network|Cortexiphan")
    static void InjectCortexiphanChaos(float DurationSeconds = 30.0f);
};