#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CortexiphanInjector.h"
#include "BishopBridge.generated.h"

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
 * BishopBridge — Walter Bishop's Bridge Between Parallel Worlds
 * Simulates 10+ to 100+ clients in-process and measures replication lag, state sync,
 * and network resilience. Produces artifacts for CI/recruiters.
 */
UCLASS()
class FRINGENETWORK_API UBishopBridge : public UObject
{
    GENERATED_BODY()

public:
    // Spawn simulated clients and run the test harness
    UFUNCTION(BlueprintCallable, Category = "Fringe Network|Bishop Bridge")
    static void SpawnSimulatedClients(int32 ClientCount = 10, float DurationMinutes = 5.0f, bool bApplyChaos = false);

    // Export replication metrics to JSON for recruiter demo
    UFUNCTION(BlueprintCallable, Category = "Fringe Network|Bishop Bridge")
    static void ExportReplicationArtifact(const FString& OutputPath = TEXT(""));

    // Get average replication lag across all clients
    UFUNCTION(BlueprintCallable, Category = "Fringe Network|Bishop Bridge")
    static float GetAverageReplicationLagMs();

    // Get sync success rate (0-1)
    UFUNCTION(BlueprintCallable, Category = "Fringe Network|Bishop Bridge")
    static float GetSyncSuccessRate();

    // Clear all simulated clients and reset state
    UFUNCTION(BlueprintCallable, Category = "Fringe Network|Bishop Bridge")
    static void ResetSimulation();
};
