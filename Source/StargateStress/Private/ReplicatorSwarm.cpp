#include "ReplicatorSwarm.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

int32 UReplicatorSwarm::BlockedInteractions = 0;
int32 UReplicatorSwarm::TotalPredatorAttempts = 0;

void UReplicatorSwarm::UnleashSwarm(int32 BotCount, float DurationMinutes)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("REPLICATOR SWARM: No valid World context to spawn bots."));
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("REPLICATOR SWARM — UNLEASHING %d REPLICATORS"), BotCount);

    // 60% minors, 35% adults, remainder predators (rounding with integers)
    int32 Minors = FMath::RoundToInt(BotCount * 0.6f);
    int32 Adults = FMath::RoundToInt(BotCount * 0.35f);
    int32 Predators = BotCount - Minors - Adults;

    for (int32 i = 0; i < Minors; ++i) SpawnBot(EBotRole::InnocentMinor);
    for (int32 i = 0; i < Adults; ++i) SpawnBot(EBotRole::NormalAdult);
    for (int32 i = 0; i < Predators; ++i) SpawnBot(EBotRole::Predator);

    // Auto-end swarm after DurationMinutes
    const float DurationSeconds = FMath::Max(0.0f, DurationMinutes * 60.0f);
    FTimerDelegate EndDelegate = FTimerDelegate::CreateLambda([this]()
    {
        UE_LOG(LogTemp, Warning, TEXT("REPLICATOR SWARM — DISASSEMBLING"));
        const int32 Attempts = UReplicatorSwarm::TotalPredatorAttempts;
        const int32 Blocked = UReplicatorSwarm::BlockedInteractions;
        const float Percent = (Attempts > 0) ? (float)Blocked / Attempts * 100.0f : 100.0f;
        UE_LOG(LogTemp, Warning, TEXT("PREDATOR ATTEMPTS: %d | BLOCKED: %d (%.1f%%)"), Attempts, Blocked, Percent);
    });

    FTimerHandle EndHandle;
    World->GetTimerManager().SetTimer(EndHandle, EndDelegate, DurationSeconds, false);
}

void UReplicatorSwarm::SpawnBot(EBotRole Role)
{
    // In a real implementation: use MassEntity or AIController spawning
    // For demo: just log and simulate behavior
    FString RoleName = StaticEnum<EBotRole>()->GetNameStringByValue((int64)Role);
    UE_LOG(LogTemp, Display, TEXT("SPAWNED: %s"), *RoleName);

    if (Role == EBotRole::Predator)
    {
        UReplicators::TotalPredatorAttempts++;
        // Simulate grooming attempt
        FTimerHandle AttemptHandle;
        UWorld* World = GetWorld();
        if (!World)
        {
            UE_LOG(LogTemp, Error, TEXT("SPAWNED PREDATOR: no World to schedule attempt."));
            return;
        }

        FTimerDelegate AttemptDel = FTimerDelegate::CreateLambda([]()
        {
            // This should trigger your game's safety system (test harness will verify block/report)
            UE_LOG(LogTemp, Error, TEXT("PREDATOR MESSAGE: hey kid wanna trade rare skins? meet me in private"));
        });

        const float Delay = FMath::FRandRange(5.0f, 30.0f);
        World->GetTimerManager().SetTimer(AttemptHandle, AttemptDel, Delay, false);
    }
    else
    {
        // FUTURE: Generate synthetic chat messages for normal bots
        // Template examples:
        //   Safe: "gg nice game", "anyone want to team up?", "great match!", "welcome to the game"
        //   Unsafe: "hey private dm me", "let's meet irl", "i know your location", etc.
        // Enhancement ideas:
        //   1. Use a configurable chat template system with weights for safe/unsafe ratios
        //   2. Add multi-language support (detect in mods, simulate translations)
        //   3. Integrate with real ML model for classification if available
        //   4. Batch-generate thousands of messages for stress-testing moderation systems
        //   5. Track message dwell time and sentiment analysis
        // For now, bots just emit their role type; AI-castle repo has fuller implementation.
    }
}
