#pragma once
#include "CoreMinimal.h"
#include "ReplicatorSwarm.generated.h"

/**
 * Simulates synthetic chat and NPC behavior at scale.
 * Used for stress-testing multiplayer systems and AI interactions.
 * 
 * Named after Stargate's Replicators - self-replicating autonomous entities
 * that multiply and stress-test game systems under load.
 */

UENUM(BlueprintType)
enum class EBotRole : uint8
{
    InnocentMinor,
    NormalAdult,
    Predator,
    Reporter
};

UCLASS()
class STARGATESTRESS_API UReplicatorSwarm : public UObject
{
    GENERATED_BODY()

public:
    // UNLEASH THE REPLICATORS
    UFUNCTION(BlueprintCallable, Category = "Stargate Stress|Replicators")
    static void UnleashSwarm(int32 BotCount = 10000, float DurationMinutes = 10.0f);

    // Manual spawn
    UFUNCTION(BlueprintCallable, Category = "Stargate Stress|Replicators")
    static void SpawnBot(EBotRole Role);

    // Stats
    static int32 BlockedInteractions;
    static int32 TotalPredatorAttempts;
};