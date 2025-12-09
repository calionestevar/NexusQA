#pragma once
#include "CoreMinimal.h"
#include "CortexiphanInjector.generated.h"

UENUM(BlueprintType)
enum class EChaosType : uint8
{
    LagSpike,
    PacketLoss,
    PacketDuplication,
    PacketReorder,
    DisconnectReconnect,
    ServerHitch,
    ClientFreeze
};

// Per-client network profile for deterministic simulation
USTRUCT(BlueprintType)
struct FNetworkProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    float BaseLatencyMs = 50.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    float JitterMs = 5.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    float PacketLossPercent = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    float DuplicationPercent = 0.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    float ReorderPercent = 2.0f;
};

UCLASS()
class FRINGENETWORK_API UCortexiphanInjector : public UObject
{
    GENERATED_BODY()

public:
    // Main entry — inject chaos for X seconds
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|Cortexiphan")
    static void InjectChaos(float DurationSeconds = 45.0f, float Intensity = 1.0f);

    // Manual triggers
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|Cortexiphan")
    static void TriggerLagSpike(float AddedLatencyMs = 800.0f);

    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|Cortexiphan")
    static void TriggerPacketLoss(float LossPercent = 30.0f, float Duration = 10.0f);

    // Per-client network simulation — set a profile for a specific client (by name or ID)
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|Cortexiphan")
    static void SetClientNetworkProfile(const FString& ClientId, const FNetworkProfile& Profile);

    // Generate deterministic jitter and packet reordering
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|Cortexiphan")
    static void SimulateJitterAndReordering(float JitterMs, float ReorderPercent, float Duration);

    // Export chaos event log to JSON file for recruiter demo
    UFUNCTION(BlueprintCallable, Category = "Fringe Guardian|Cortexiphan")
    static void ExportChaosArtifact(const FString& OutputPath = TEXT(""));
};