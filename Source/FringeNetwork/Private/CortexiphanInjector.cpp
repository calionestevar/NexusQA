#include "CortexiphanInjector.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Json.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Containers/Map.h"
#include "Nexus/Palantir/Public/PalantirTrace.h"

// Global chaos event log for artifact export
static TArray<TPair<FString, FString>> GChaosEventLog;
static FCriticalSection GChaosLogMutex;

// Per-client network profiles (mutable map for runtime selection)
static TMap<FString, FNetworkProfile> GClientNetworkProfiles;

static void ChaosLog(const FString& Msg)
{
    UE_LOG_TRACE(LogTemp, Error, TEXT("CORTEXIPHAN: %s"), *Msg);
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("CORTEXIPHAN: ") + Msg);

    // Record to event log for artifact export
    FScopeLock Lock(&GChaosLogMutex);
    FString TraceID = FPalantirTrace::GetCurrentTraceID();
    FString LogEntry = TraceID.IsEmpty() ? Msg : FString::Printf(TEXT("[%s] %s"), *TraceID, *Msg);
    GChaosEventLog.Add(TPair<FString, FString>(FDateTime::Now().ToString(), LogEntry));
    
    // Add breadcrumb if trace is active
    if (!TraceID.IsEmpty())
    {
        PALANTIR_BREADCRUMB(TEXT("ChaosEvent"), Msg);
    }
}

void UCortexiphanInjector::InjectChaos(float DurationSeconds, float Intensity)
{
    // Get world context - use GEngine since this is a static-like operation
    UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
    if (!World)
    {
        ChaosLog(TEXT("No world context for CortexiphanInjector"));
        return;
    }

    ChaosLog(FString::Printf(TEXT("INJECTING CORTEXIPHAN — CHAOS FOR %.0f SECONDS — INTENSITY %.1f"), DurationSeconds, Intensity));

    // Store time left in a shared value so the lambda can safely access it after this function returns.
    TSharedRef<float, ESPMode::ThreadSafe> TimeLeft = MakeShared<float, ESPMode::ThreadSafe>(DurationSeconds);

    // Use shared refs for timer handles to keep them alive across lambda lifetimes
    TSharedRef<FTimerHandle, ESPMode::ThreadSafe> ChaosTimerHandle = MakeShared<FTimerHandle, ESPMode::ThreadSafe>();
    TSharedRef<FTimerHandle, ESPMode::ThreadSafe> EndTimerHandle = MakeShared<FTimerHandle, ESPMode::ThreadSafe>();

    // Periodic chaos tick — safe lambda captures (no references to stack locals or this)
    World->GetTimerManager().SetTimer(*ChaosTimerHandle, FTimerDelegate::CreateLambda([TimeLeft, Intensity]()
    {
        if (*TimeLeft <= 0.0f) return;

        float Roll = FMath::FRand();
        float Adjusted = Roll * Intensity;

        if (Adjusted < 0.4f)
        {
            UCortexiphanInjector::TriggerLagSpike(400 + FMath::RandRange(0, 800));
        }
        else if (Adjusted < 0.7f)
        {
            UCortexiphanInjector::TriggerPacketLoss(25 + FMath::RandRange(0, 40), 5.0f);
        }
        else if (Adjusted < 0.85f)
        {
            UCortexiphanInjector::TriggerLagSpike(1500);
        }
        else
        {
            // Simulate extreme client impact without blocking the game thread.
            ChaosLog(TEXT("TOTAL REALITY BREACH — CLIENT FREEZE (simulated)"));
            // TODO: Consider implementing an actual NetDriver simulation here instead
            // of blocking the thread. Hook into the UNetDriver packet pipeline for
            // deterministic latency/packet-loss simulation in CI runs.
        }

        *TimeLeft -= 3.0f;
    }), 3.0f, true);

    // End chaos after the requested duration — clear the periodic timer safely.
    World->GetTimerManager().SetTimer(*EndTimerHandle, FTimerDelegate::CreateLambda([ChaosTimerHandle, World]()
    {
        if (World && World->GetTimerManager().IsTimerActive(*ChaosTimerHandle))
        {
            World->GetTimerManager().ClearTimer(*ChaosTimerHandle);
            ChaosLog(TEXT("CORTEXIPHAN EFFECT SUBSIDING — RETURNING TO BASELINE"));
        }
    }), DurationSeconds, false);
}

void UCortexiphanInjector::TriggerLagSpike(float AddedLatencyMs)
{
    ChaosLog(FString::Printf(TEXT("LAG SPIKE +%.0fms"), AddedLatencyMs));
    // In a real game: modify NetDriver->LagCompensation or use console commands
    if (GEngine)
    {
        UWorld* World = GEngine->GetCurrentPlayWorld();
        APlayerController* PC = World ? GEngine->GetFirstLocalPlayerController(World) : nullptr;
        if (PC && World)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("+%.0fms LAG"), AddedLatencyMs));
        }
    }
}

void UCortexiphanInjector::TriggerPacketLoss(float LossPercent, float Duration)
{
    ChaosLog(FString::Printf(TEXT("PACKET LOSS %.0f%% FOR %.0fs"), LossPercent, Duration));
    // Real implementation would hook into NetDriver
    GEngine->AddOnScreenDebugMessage(-1, Duration + 1.0f, FColor::Purple, FString::Printf(TEXT("PACKET LOSS %.0f%%"), LossPercent));
}

void UCortexiphanInjector::SetClientNetworkProfile(const FString& ClientId, const FNetworkProfile& Profile)
{
    GClientNetworkProfiles.FindOrAdd(ClientId) = Profile;
    ChaosLog(FString::Printf(TEXT("SET PROFILE FOR CLIENT %s: latency=%.0fms jitter=%.0fms loss=%.1f%% reorder=%.1f%%"),
        *ClientId, Profile.BaseLatencyMs, Profile.JitterMs, Profile.PacketLossPercent, Profile.ReorderPercent));
}

void UCortexiphanInjector::SimulateJitterAndReordering(float JitterMs, float ReorderPercent, float Duration)
{
    ChaosLog(FString::Printf(TEXT("JITTER %.0fms + REORDER %.0f%% FOR %.0fs"), JitterMs, ReorderPercent, Duration));

    UWorld* World = GWorld ? GWorld : (GEngine && GEngine->GetCurrentPlayWorld() ? GEngine->GetCurrentPlayWorld() : nullptr);
    if (!World) return;

    TSharedRef<float, ESPMode::ThreadSafe> TimeLeft = MakeShared<float, ESPMode::ThreadSafe>(Duration);
    FTimerHandle JitterHandle;

    // Periodic jitter injection
    World->GetTimerManager().SetTimer(JitterHandle, FTimerDelegate::CreateLambda([TimeLeft, JitterMs, ReorderPercent]()
    {
        if (*TimeLeft <= 0.0f) return;

        // Simulate jitter variance
        float ActualJitter = JitterMs * (0.5f + FMath::FRand());
        if (FMath::FRand() < ReorderPercent / 100.0f)
        {
            ChaosLog(FString::Printf(TEXT("PACKET REORDER: jitter=%.0fms"), ActualJitter));
        }

        *TimeLeft -= 1.0f;
    }), 1.0f, true);

    // Stop jitter injection after duration
    // Use a shared ref to keep the handle alive across lambda lifetime
    TSharedRef<FTimerHandle, ESPMode::ThreadSafe> JitterHandlePtr = MakeShared<FTimerHandle, ESPMode::ThreadSafe>(JitterHandle);
    FTimerHandle StopHandle;
    World->GetTimerManager().SetTimer(StopHandle, FTimerDelegate::CreateLambda([World, JitterHandlePtr]()
    {
        if (World)
        {
            World->GetTimerManager().ClearTimer(*JitterHandlePtr);
            ChaosLog(TEXT("JITTER/REORDER SUBSIDING"));
        }
    }), Duration, false);
}

void UCortexiphanInjector::ExportChaosArtifact(const FString& OutputPath)
{
    TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);

    // Add metadata
    Root->SetStringField(TEXT("timestamp"), FDateTime::Now().ToString());
    Root->SetStringField(TEXT("title"), TEXT("CORTEXIPHAN CHAOS SIMULATION LOG"));

    // Add chaos events array
    TArray<TSharedPtr<FJsonValue>> EventsArray;
    {
        FScopeLock Lock(&GChaosLogMutex);
        for (const auto& Event : GChaosEventLog)
        {
            TSharedPtr<FJsonObject> EventObj = MakeShareable(new FJsonObject);
            EventObj->SetStringField(TEXT("time"), Event.Key);
            EventObj->SetStringField(TEXT("event"), Event.Value);
            EventsArray.Add(MakeShareable(new FJsonValueObject(EventObj)));
        }
    }
    Root->SetArrayField(TEXT("events"), EventsArray);

    // Add client profiles
    TArray<TSharedPtr<FJsonValue>> ProfilesArray;
    for (const auto& Pair : GClientNetworkProfiles)
    {
        TSharedPtr<FJsonObject> ProfileObj = MakeShareable(new FJsonObject);
        ProfileObj->SetStringField(TEXT("client"), Pair.Key);
        ProfileObj->SetNumberField(TEXT("baseLatencyMs"), Pair.Value.BaseLatencyMs);
        ProfileObj->SetNumberField(TEXT("jitterMs"), Pair.Value.JitterMs);
        ProfileObj->SetNumberField(TEXT("packetLossPercent"), Pair.Value.PacketLossPercent);
        ProfileObj->SetNumberField(TEXT("duplicationPercent"), Pair.Value.DuplicationPercent);
        ProfileObj->SetNumberField(TEXT("reorderPercent"), Pair.Value.ReorderPercent);
        ProfilesArray.Add(MakeShareable(new FJsonValueObject(ProfileObj)));
    }
    Root->SetArrayField(TEXT("clientProfiles"), ProfilesArray);
    Root->SetNumberField(TEXT("eventCount"), EventsArray.Num());

    // Serialize to file
    FString OutputFile = OutputPath;
    if (OutputFile.IsEmpty())
    {
        OutputFile = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("NexusReports/CortexiphanChaosLog.json"));
    }
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(*OutputFile));

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

    if (FFileHelper::SaveStringToFile(JsonString, *OutputFile))
    {
        ChaosLog(FString::Printf(TEXT("CHAOS ARTIFACT EXPORTED → %s"), *OutputFile));
    }
    else
    {
        ChaosLog(FString::Printf(TEXT("FAILED TO WRITE CHAOS ARTIFACT → %s"), *OutputFile));
    }
}