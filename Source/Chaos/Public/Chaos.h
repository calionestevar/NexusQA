#pragma once
#include "CoreMinimal.h"
#include "Chaos.generated.h"

UCLASS()
class UChaos : public UObject
{
    GENERATED_BODY()

public:
    // Master command — runs the full Observer Network
    UFUNCTION(BlueprintCallable, Category = "Chaos")
    static void ActivateObserverNetwork();

    // Individual Observers
    UFUNCTION(BlueprintCallable, Category = "Chaos|Observers")
    static void RunObserverNetworkTests(const FString& PrimaryServer);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Parallel Realms")
    static void TestParallelRealms(const TArray<FString>& RegionURLs);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Cortexiphan")
    static void InjectCortexiphanChaos(float DurationSeconds = 30.0f);
};