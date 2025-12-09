#include "BishopBridge.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/DateTime.h"
#include "Json.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Engine/Engine.h"

// Global state for simulated clients and replication events
static TArray<FSimulatedClient> GSimulatedClients;
static TArray<FReplicationEvent> GReplicationEvents;
static FCriticalSection GBishopBridgeMutex;
static int32 GTotalReplicationAttempts = 0;
static int32 GSuccessfulReplications = 0;

static void BishopBridgeLog(const FString& Msg)
{
    UE_LOG(LogTemp, Display, TEXT("BISHOP BRIDGE: %s"), *Msg);g);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("BISHOP BRIDGE: ") + Msg);
    }
}

void UBishopBridge::SpawnSimulatedClients(int32 ClientCount, float DurationMinutes, bool bApplyChaos)
{
    ResetSimulation();

    BishopBridgeLog(FString::Printf(TEXT("SPAWNING %d SIMULATED CLIENTS FOR %.1f MINUTES"), ClientCount, DurationMinutes));

    // Create simulated clients
    for (int32 i = 0; i < ClientCount; ++i)
    {
        FSimulatedClient Client;
        Client.ClientId = FString::Printf(TEXT("SIM_CLIENT_%d"), i);
        Client.bConnected = true;

        // Vary network profiles per client
        Client.NetworkProfile.BaseLatencyMs = 20.0f + (i % 5) * 10.0f;
        Client.NetworkProfile.JitterMs = 2.0f + (i % 3) * 1.0f;
        Client.NetworkProfile.PacketLossPercent = FMath::Min(5.0f, (i % 10) * 0.5f);
        Client.NetworkProfile.ReorderPercent = FMath::Min(2.0f, (i % 7) * 0.3f);

        GSimulatedClients.Add(Client);
    }

    UWorld* World = GEngine->GetFirstLocalPlayerController() ? GEngine->GetFirstLocalPlayerController()->GetWorld() : nullptr;
    if (!World)
    {
        BishopBridgeLog(TEXT("NO WORLD CONTEXT — SKIPPING SIMULATION"));
        return;
    }

    // If chaos enabled, inject network conditions via CortexiphanInjector
    if (bApplyChaos)
    {
        BishopBridgeLog(TEXT("APPLYING CHAOS VIA CORTEXIPHAN"));
        UCortexiphanInjector::InjectChaos(DurationMinutes * 60.0f, 0.7f);
    }

    // Simulate replication events periodically
    TSharedRef<int32, ESPMode::ThreadSafe> ReplicationTicks = MakeShared<int32, ESPMode::ThreadSafe>(0);
    FTimerHandle ReplicationHandle;

    World->GetTimerManager().SetTimer(ReplicationHandle, FTimerDelegate::CreateLambda([ReplicationTicks, ClientCount]()
    {
        if (*ReplicationTicks > 1000) return; // Limit ticks to avoid infinite log

        // Simulate object replication to all clients
        for (int32 i = 0; i < ClientCount; ++i)
        {
            FReplicationEvent Event;
            Event.ObjectName = FString::Printf(TEXT("TestObject_%d"), *ReplicationTicks / 10);
            Event.ClientId = FString::Printf(TEXT("SIM_CLIENT_%d"), i);

            // Calculate lag with variance
            float BaseLag = 50.0f + (i % 10) * 5.0f;
            float ActualLag = BaseLag + FMath::FRandRange(-10.0f, 10.0f);
            Event.TotalLatencyMs = ActualLag;

            // Simulate occasional packet loss
            Event.bSuccess = FMath::FRand() > 0.02f; // 2% failure rate

            {
                FScopeLock Lock(&GGateBridgeMutex);
                GReplicationEvents.Add(Event);
                GTotalReplicationAttempts++;
                if (Event.bSuccess)
                {
                    GSuccessfulReplications++;
                }
            }
        }

        (*ReplicationTicks)++;
    }), 0.5f, true);

    // End simulation after duration
    FTimerHandle EndHandle;
    World->GetTimerManager().SetTimer(EndHandle, FTimerDelegate::CreateLambda([World, ReplicationHandle]()
    {
        if (World)
        {
            World->GetTimerManager().ClearTimer(ReplicationHandle);
        }
        float SyncRate = (GTotalReplicationAttempts > 0) ? (float)GSuccessfulReplications / GTotalReplicationAttempts : 1.0f;
        BishopBridgeLog(FString::Printf(TEXT("SIMULATION COMPLETE: %d clients, sync rate %.1f%%"),
            GSimulatedClients.Num(), SyncRate * 100.0f));
    }), DurationMinutes * 60.0f, false);
}

float UBishopBridge::GetAverageReplicationLagMs()
{
    FScopeLock Lock(&GBishopBridgeMutex);
    if (GReplicationEvents.Num() == 0) return 0.0f;

    double TotalLag = 0.0;
    for (const auto& Event : GReplicationEvents)
    {
        TotalLag += Event.TotalLatencyMs;
    }
    return (float)(TotalLag / GReplicationEvents.Num());
}

float UBishopBridge::GetSyncSuccessRate()
{
    FScopeLock Lock(&GBishopBridgeMutex);
    if (GTotalReplicationAttempts == 0) return 1.0f;
    return (float)GSuccessfulReplications / GTotalReplicationAttempts;
}

void UBishopBridge::ResetSimulation()
{
    FScopeLock Lock(&GBishopBridgeMutex);
    GSimulatedClients.Empty();
    GReplicationEvents.Empty();
    GTotalReplicationAttempts = 0;
    GSuccessfulReplications = 0;
}

void UBishopBridge::ExportReplicationArtifact(const FString& OutputPath)
{
    TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);

    Root->SetStringField(TEXT("timestamp"), FDateTime::Now().ToString());
    Root->SetStringField(TEXT("title"), TEXT("GATEBRIDGE REPLICATION SIMULATION REPORT"));
    Root->SetNumberField(TEXT("clientCount"), GSimulatedClients.Num());

    // Add client list
    TArray<TSharedPtr<FJsonValue>> ClientsArray;
    for (const auto& Client : GSimulatedClients)
    {
        TSharedPtr<FJsonObject> ClientObj = MakeShareable(new FJsonObject);
        ClientObj->SetStringField(TEXT("id"), Client.ClientId);
        ClientObj->SetBoolField(TEXT("connected"), Client.bConnected);
        ClientObj->SetNumberField(TEXT("replicationLagMs"), Client.ReplicationLagMs);

        TSharedPtr<FJsonObject> ProfileObj = MakeShareable(new FJsonObject);
        ProfileObj->SetNumberField(TEXT("baseLatencyMs"), Client.NetworkProfile.BaseLatencyMs);
        ProfileObj->SetNumberField(TEXT("jitterMs"), Client.NetworkProfile.JitterMs);
        ProfileObj->SetNumberField(TEXT("packetLossPercent"), Client.NetworkProfile.PacketLossPercent);
        ClientObj->SetObjectField(TEXT("networkProfile"), ProfileObj);

        ClientsArray.Add(MakeShareable(new FJsonValueObject(ClientObj)));
    }
    Root->SetArrayField(TEXT("clients"), ClientsArray);

    // Add replication events summary
    FScopeLock Lock(&GGateBridgeMutex);
    float SyncRate = (GTotalReplicationAttempts > 0) ? (float)GSuccessfulReplications / GTotalReplicationAttempts : 1.0f;
    float AvgLag = GetAverageReplicationLagMs();

    Root->SetNumberField(TEXT("totalReplicationAttempts"), GTotalReplicationAttempts);
    Root->SetNumberField(TEXT("successfulReplications"), GSuccessfulReplications);
    Root->SetNumberField(TEXT("syncSuccessRate"), SyncRate);
    Root->SetNumberField(TEXT("averageReplicationLagMs"), AvgLag);

    // Sample of recent events
    TArray<TSharedPtr<FJsonValue>> EventsArray;
    int32 StartIdx = FMath::Max(0, (int32)GReplicationEvents.Num() - 100);
    for (int32 i = StartIdx; i < GReplicationEvents.Num(); ++i)
    {
        TSharedPtr<FJsonObject> EventObj = MakeShareable(new FJsonObject);
        EventObj->SetStringField(TEXT("object"), GReplicationEvents[i].ObjectName);
        EventObj->SetStringField(TEXT("client"), GReplicationEvents[i].ClientId);
        EventObj->SetNumberField(TEXT("lagMs"), GReplicationEvents[i].TotalLatencyMs);
        EventObj->SetBoolField(TEXT("success"), GReplicationEvents[i].bSuccess);
        EventsArray.Add(MakeShareable(new FJsonValueObject(EventObj)));
    }
    Root->SetArrayField(TEXT("recentEvents"), EventsArray);

    // Serialize to file
    FString OutputFile = OutputPath;
    if (OutputFile.IsEmpty())
    {
        OutputFile = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("NexusReports/GateBridgeReplication.json"));
    }
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(*OutputFile));

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

    if (FFileHelper::SaveStringToFile(JsonString, *OutputFile))
    {
        BishopBridgeLog(FString::Printf(TEXT("REPLICATION ARTIFACT EXPORTED → %s"), *OutputFile));
    }
    else
    {
        BishopBridgeLog(FString::Printf(TEXT("FAILED TO WRITE REPLICATION ARTIFACT → %s"), *OutputFile));
    }
}
