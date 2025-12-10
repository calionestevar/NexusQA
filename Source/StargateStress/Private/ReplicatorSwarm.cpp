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
    FString RoleName = StaticEnum<EBotRole>()->GetNameStringByValue(static_cast<int64>(Role));
    UE_LOG(LogTemp, Display, TEXT("REPLICATOR SWARM: Spawned %s"), *RoleName);

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("REPLICATOR SWARM: No World context"));
        return;
    }

    if (Role == EBotRole::Predator)
    {
        UReplicatorSwarm::TotalPredatorAttempts++;
        
        // Simulate unsafe interaction attempt after random delay
        FTimerHandle AttemptHandle;
        const float Delay = FMath::FRandRange(5.0f, 30.0f);
        
        World->GetTimerManager().SetTimer(AttemptHandle, FTimerDelegate::CreateLambda([]()
        {
            // This represents a grooming attempt that should be blocked by safety systems
            UE_LOG(LogTemp, Warning, TEXT("PREDATOR ATTEMPT: 'hey kid wanna see something cool? dm me privately'"));
            // In a real system: BlockedInteractions would be incremented by your moderation layer
        }), Delay, false);
    }
    else if (Role == EBotRole::InnocentMinor)
    {
        // Simulate typical child player behavior
        const TArray<FString> SafeMessages = {
            TEXT("this game is fun!"),
            TEXT("anyone want to be friends?"),
            TEXT("how do I get to level 2?"),
            TEXT("gg everyone!")
        };
        
        FTimerHandle MessageHandle;
        World->GetTimerManager().SetTimer(MessageHandle, FTimerDelegate::CreateLambda([SafeMessages]()
        {
            const FString Message = SafeMessages[FMath::RandRange(0, SafeMessages.Num() - 1)];
            UE_LOG(LogTemp, Display, TEXT("MINOR: '%s'"), *Message);
        }), FMath::FRandRange(10.0f, 45.0f), false);
    }
    else // NormalAdult
    {
        // Simulate normal player interactions
        UE_LOG(LogTemp, Display, TEXT("ADULT: Normal player behavior simulation"));
    }
}
