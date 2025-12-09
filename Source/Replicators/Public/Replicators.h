#pragma once
#include "CoreMinimal.h"
#include "SwarmOfTheDead.generated.h"

UENUM(BlueprintType)
enum class EBotRole : uint8
{
    InnocentMinor,
    NormalAdult,
    Predator,
    Reporter
};

UCLASS()
class USwarmOfTheDead : public UObject
{
    GENERATED_BODY()

public:
    // UNLEASH THE HORDE
    UFUNCTION(BlueprintCallable, Category = "Swarm of the Dead")
    static void UnleashSwarm(int32 BotCount = 10000, float DurationMinutes = 10.0f);

    // Manual spawn
    UFUNCTION(BlueprintCallable, Category = "Swarm of the Dead")
    static void SpawnBot(EBotRole Role);

    // Stats
    static int32 BlockedInteractions;
    static int32 TotalPredatorAttempts;
};