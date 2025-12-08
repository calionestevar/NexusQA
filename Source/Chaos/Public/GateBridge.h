#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CortexiphanInjector.h"
#include "GateBridge.generated.h"

// Lightweight simulated client for in-process replication testing
USTRUCT(BlueprintType)
struct FSimulatedClient
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString ClientId;

    UPROPERTY(BlueprintReadWrite)
    float ReplicationLagMs = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bConnected = true;

    UPROPERTY(BlueprintReadWrite)
    FNetworkProfile NetworkProfile;
};

// Replication event for tracking state synchronization
USTRUCT(BlueprintType)
struct FReplicationEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString ObjectName;

    UPROPERTY(BlueprintReadWrite)
    FString ClientId;

    UPROPERTY(BlueprintReadWrite)
    float TotalLatencyMs = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bSuccess = true;
};

/**
 * GateBridge — Stargate GateBridge Multiplayer Test Harness
 * Simulates 10+ to 100+ clients in-process and measures replication lag, state sync,
 * and network resilience. Produces artifacts for CI/recruiters.
 */
UCLASS()
class UGateBridge : public UObject
{
    GENERATED_BODY()

public:
    // Spawn simulated clients and run the test harness
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|GateBridge")
    static void SpawnSimulatedClients(int32 ClientCount = 10, float DurationMinutes = 5.0f, bool bApplyChaos = false);

    // Export replication metrics to JSON for recruiter demo
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|GateBridge")
    static void ExportReplicationArtifact(const FString& OutputPath = TEXT(""));

    // Get average replication lag across all clients
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|GateBridge")
    static float GetAverageReplicationLagMs();

    // Get sync success rate (0-1)
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|GateBridge")
    static float GetSyncSuccessRate();

    // Clear all simulated clients and reset state
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|GateBridge")
    static void ResetSimulation();
};
