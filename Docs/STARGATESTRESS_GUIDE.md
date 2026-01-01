# StargateStress — Load Testing & Bot Simulation

**StargateStress** provides tools for load testing, simulating concurrent players, testing bot AI, and validating system stability under stress. Named after Stargate's military command structure that enforces protocols under pressure, this module ensures your game handles peak load gracefully.

---

## Overview

### Core Capabilities

| Feature | Purpose | Use Case |
|---------|---------|----------|
| **Concurrent User Simulation** | Spawn N virtual players | Test max player limits, server scalability |
| **Bot Behavior Patterns** | Realistic player movement/actions | Test pathfinding, combat AI, map coverage |
| **Load Ramping** | Gradually increase players | Find breaking point, measure degradation curve |
| **Safety Validation** | Ensure stability under load | Prevent crashes, validate resource management |
| **Stress Metrics** | Track system behavior under load | CPU/memory/network usage |

---

## Quick Start

### 1. Add StargateStress to Your Project

**In your `.Build.cs` file:**

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "StargateStress",
    "AIModule",
    "GameplayTasks"
});
```

### 2. Configure Load Test

**In DefaultEngine.ini:**

```ini
[/Script/StargateStress.StargateStressConfig]
; Load test settings
InitialPlayerCount=10
MaxPlayerCount=100
PlayerSpawnRatePerSecond=5.0
BotBehaviorType=Realistic
BotSpawnMap=/Game/Maps/LoadTest
```

### 3. Run Load Test

```cpp
#include "StargateStress/Public/StargateStress.h"

void ALoadTestGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Spawn initial bots
    UStargateStress::SpawnBotsForLoadTest(10);
    
    // Gradually ramp up
    GetWorld()->GetTimerManager().SetTimer(
        LoadRampHandle,
        [this]() { RampUpLoad(); },
        1.0f,  // Every 1 second
        true   // Loop
    );
}

void ALoadTestGameMode::RampUpLoad()
{
    static int32 CurrentCount = 10;
    if (CurrentCount < 100) {
        UStargateStress::SpawnBotsForLoadTest(++CurrentCount);
    }
}
```

---

## API Reference

### Spawn Bots for Load Testing

```cpp
void UStargateStress::SpawnBotsForLoadTest(
    int32 BotCount,
    const FString& BehaviorType = TEXT("Realistic")
);
```

Creates N bot players with specified behavior pattern.

**Parameters:**
- `BotCount` — Number of bots to spawn
- `BehaviorType` — "Realistic" (player-like), "Aggressive" (combat-heavy), or "Passive" (AFK)

**Example:**

```cpp
// Spawn 50 realistic bots
UStargateStress::SpawnBotsForLoadTest(50, TEXT("Realistic"));

// Spawn 100 aggressive bots for PvP stress test
UStargateStress::SpawnBotsForLoadTest(100, TEXT("Aggressive"));

// Spawn 200 passive bots (minimal CPU) for connection test
UStargateStress::SpawnBotsForLoadTest(200, TEXT("Passive"));
```

**What it does:**
- Creates bot characters at predefined spawn points
- Assigns AI controllers with configured behavior
- Distributes bots across map to avoid clustering
- Logs bot creation progress

---

### Run Load Ramp Test

```cpp
void UStargateStress::RunLoadRampTest(
    int32 StartCount,
    int32 EndCount,
    int32 StepSize = 10,
    float DurationPerStepSeconds = 30.0f
);
```

Gradually increases player count to find breaking point.

**Parameters:**
- `StartCount` — Initial player count
- `EndCount` — Maximum player count to reach
- `StepSize` — How many players to add per step (default 10)
- `DurationPerStepSeconds` — How long to run at each level (default 30 seconds)

**Example:**

```cpp
// Ramp from 10 to 100 players, adding 10 every 30 seconds
UStargateStress::RunLoadRampTest(10, 100, 10, 30.0f);

// Ramp aggressively: 0 to 50 players, adding 5 every 5 seconds
UStargateStress::RunLoadRampTest(0, 50, 5, 5.0f);
```

**What it does:**
- Spawns `StartCount` bots initially
- Waits for `DurationPerStepSeconds`
- Adds `StepSize` bots
- Repeats until `EndCount` reached
- Collects metrics at each step

---

### Configure Bot Behavior

```cpp
void UStargateStress::SetBotBehaviorPattern(
    const FString& PatternName,
    const FBotBehaviorConfig& Config
);
```

Define how bots act during load test.

**Behavior Patterns:**

```cpp
struct FBotBehaviorConfig
{
    // Movement
    float MoveSpeed = 500.0f;                      // Units per second
    bool bPatrolWaypoints = true;                  // Follow map waypoints
    float PatrolChangeIntervalSeconds = 30.0f;    // How often to change destination
    
    // Combat (if applicable)
    bool bEngageEnemies = true;
    float CombatUpdateRateSeconds = 0.1f;
    
    // Network
    bool bSendPositionUpdates = true;
    float PositionUpdateRateSeconds = 0.1f;
    
    // AI Complexity
    bool bUsePathfinding = true;
    bool bDetectObstacles = true;
};
```

**Example:**

```cpp
// High-traffic realistic behavior
FBotBehaviorConfig RealisticConfig;
RealisticConfig.MoveSpeed = 600.0f;
RealisticConfig.bPatrolWaypoints = true;
RealisticConfig.PatrolChangeIntervalSeconds = 15.0f;
RealisticConfig.bEngageEnemies = true;
RealisticConfig.bSendPositionUpdates = true;
RealisticConfig.PositionUpdateRateSeconds = 0.05f;  // High frequency updates
UStargateStress::SetBotBehaviorPattern(TEXT("HighTraffic"), RealisticConfig);

// Low-CPU passive behavior (connection only)
FBotBehaviorConfig PassiveConfig;
PassiveConfig.MoveSpeed = 0.0f;
PassiveConfig.bPatrolWaypoints = false;
PassiveConfig.bEngageEnemies = false;
PassiveConfig.bSendPositionUpdates = false;
PassiveConfig.bUsePathfinding = false;
PassiveConfig.bDetectObstacles = false;
UStargateStress::SetBotBehaviorPattern(TEXT("Passive"), PassiveConfig);
```

---

### Get Load Test Metrics

```cpp
struct FStressMetrics
{
    int32 TotalBots = 0;
    float AverageCPUUsagePercent = 0.0f;
    float PeakMemoryMb = 0.0f;
    float NetworkBandwidthMbps = 0.0f;
    float AverageFrameTimeMs = 0.0f;
    bool bSystemStable = true;
};

FStressMetrics UStargateStress::GetCurrentMetrics();
```

Query current system load metrics.

**Example:**

```cpp
FStressMetrics Metrics = UStargateStress::GetCurrentMetrics();

UE_LOG(LogTemp, Display, TEXT("Load Test Metrics:"));
UE_LOG(LogTemp, Display, TEXT("  Bots: %d"), Metrics.TotalBots);
UE_LOG(LogTemp, Display, TEXT("  CPU: %.1f%%"), Metrics.AverageCPUUsagePercent);
UE_LOG(LogTemp, Display, TEXT("  Memory: %.0f MB"), Metrics.PeakMemoryMb);
UE_LOG(LogTemp, Display, TEXT("  Frame Time: %.2f ms"), Metrics.AverageFrameTimeMs);
UE_LOG(LogTemp, Display, TEXT("  Stable: %s"), Metrics.bSystemStable ? TEXT("✅ Yes") : TEXT("❌ No"));
```

---

### Validate System Under Load

```cpp
bool UStargateStress::ValidateSystemStability();
```

Checks whether system remains stable under current load.

**Example:**

```cpp
// After reaching max load, validate stability
if (!UStargateStress::ValidateSystemStability()) {
    UE_LOG(LogTemp, Error, TEXT("❌ System unstable under load!"));
    return false;
}

UE_LOG(LogTemp, Display, TEXT("✅ System stable at max load"));
return true;
```

**Checks:**
- No crashes or assertion failures
- Frame rate above minimum threshold
- Memory usage below budget
- Network bandwidth acceptable
- No deadlocks or timeouts

---

### Kill All Test Bots

```cpp
void UStargateStress::CleanupAllBots();
```

Removes all spawned bots and cleans up resources.

**Example:**

```cpp
void ALoadTestGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
    Super::EndPlay(Reason);
    
    // Clean up all test bots
    UStargateStress::CleanupAllBots();
    
    UE_LOG(LogTemp, Display, TEXT("Load test cleanup complete"));
}
```

---

## Integration Examples

### Example 1: Progressive Load Test

```cpp
// LoadTestGameMode.h
UCLASS()
class MYGAME_API ALoadTestGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
    void OnLoadStepComplete();
    
    int32 CurrentPlayerCount = 0;
    float CurrentStepTime = 0.0f;
};
```

```cpp
// LoadTestGameMode.cpp
#include "LoadTestGameMode.h"
#include "StargateStress/Public/StargateStress.h"
#include "ArgusLens/Public/ArgusLens.h"

void ALoadTestGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Configure monitoring
    UArgusLens::StartPerformanceMonitoring(600.0f);  // 10 minute test
    
    // Start with 10 players
    CurrentPlayerCount = 10;
    UStargateStress::SpawnBotsForLoadTest(CurrentPlayerCount);
    CurrentStepTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("🔥 Load test starting with %d players"), CurrentPlayerCount);
}

void ALoadTestGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CurrentStepTime += DeltaTime;
    
    // Increase players every 30 seconds, up to 100
    if (CurrentStepTime >= 30.0f && CurrentPlayerCount < 100)
    {
        CurrentPlayerCount = FMath::Min(CurrentPlayerCount + 10, 100);
        UStargateStress::SpawnBotsForLoadTest(CurrentPlayerCount);
        
        FStressMetrics Metrics = UStargateStress::GetCurrentMetrics();
        UE_LOG(LogTemp, Warning, TEXT("Ramping up to %d players — Frame: %.1fms, Memory: %.0fMB"),
            CurrentPlayerCount, Metrics.AverageFrameTimeMs, Metrics.PeakMemoryMb);
        
        CurrentStepTime = 0.0f;
    }
}

void ALoadTestGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
    Super::EndPlay(Reason);
    
    // Stop monitoring
    UArgusLens::StopPerformanceMonitoring();
    
    // Clean up bots
    UStargateStress::CleanupAllBots();
    
    // Check stability
    if (UStargateStress::ValidateSystemStability()) {
        UE_LOG(LogTemp, Display, TEXT("✅ Load test PASSED"));
    } else {
        UE_LOG(LogTemp, Error, TEXT("❌ Load test FAILED - system unstable"));
    }
    
    // Export results
    UArgusLens::ExportPerformanceArtifact();
}
```

---

### Example 2: Load Test with Safety Validation

```cpp
// SafetyUnderLoadTest.cpp
NEXUS_TEST(FSafetyUnderLoadTest, "Stress.Safety.MaxPlayers", ETestPriority::Critical)
{
    // Spawn 50 bots aggressively
    UStargateStress::SpawnBotsForLoadTest(50, TEXT("Aggressive"));
    
    // Let them run for 60 seconds
    FPlatformProcess::Sleep(60.0f);
    
    // Check system didn't crash or hang
    FStressMetrics Metrics = UStargateStress::GetCurrentMetrics();
    
    if (!Metrics.bSystemStable) {
        UE_LOG(LogTemp, Error, TEXT("❌ System crashed under load"));
        UStargateStress::CleanupAllBots();
        return false;
    }
    
    // Verify reasonable resource usage
    if (Metrics.PeakMemoryMb > 4096.0f) {
        UE_LOG(LogTemp, Error, TEXT("❌ Memory exceeded budget: %.0f MB"), Metrics.PeakMemoryMb);
        UStargateStress::CleanupAllBots();
        return false;
    }
    
    UStargateStress::CleanupAllBots();
    UE_LOG(LogTemp, Display, TEXT("✅ Safety validation PASSED at %d concurrent players"), Metrics.TotalBots);
    return true;
}
```

---

### Example 3: Find Breaking Point

```cpp
// BreakingPointTest.cpp
NEXUS_TEST(FBreakingPointTest, "Stress.Performance.MaxCapacity", ETestPriority::High)
{
    // Configure realistic bot behavior
    FBotBehaviorConfig BotConfig;
    BotConfig.MoveSpeed = 500.0f;
    BotConfig.bPatrolWaypoints = true;
    BotConfig.bSendPositionUpdates = true;
    BotConfig.PositionUpdateRateSeconds = 0.1f;
    UStargateStress::SetBotBehaviorPattern(TEXT("Realistic"), BotConfig);
    
    // Start load ramp test: 0 → 200 players, +20 every 20 seconds
    UStargateStress::RunLoadRampTest(0, 200, 20, 20.0f);
    
    // After ramp, analyze results
    FStressMetrics Metrics = UStargateStress::GetCurrentMetrics();
    
    // Report breaking point
    if (!Metrics.bSystemStable) {
        UE_LOG(LogTemp, Warning, TEXT("⚠️  Breaking point: ~%d players"), Metrics.TotalBots);
    } else {
        UE_LOG(LogTemp, Display, TEXT("✅ System stable at %d concurrent players"), Metrics.TotalBots);
    }
    
    UStargateStress::CleanupAllBots();
    return true;
}
```

---

## Configuration Guide

### DefaultEngine.ini

```ini
[/Script/StargateStress.StargateStressConfig]
; Initial spawn count
InitialPlayerCount=10

; Maximum concurrent players to test
MaxPlayerCount=100

; How fast to spawn bots (per second)
PlayerSpawnRatePerSecond=5.0

; Default behavior pattern ("Realistic", "Aggressive", "Passive")
BotBehaviorType=Realistic

; Map to spawn bots on (must have waypoints/spawners)
BotSpawnMap=/Game/Maps/LoadTest

; Safety thresholds
MaxMemoryBudgetMb=4096
MinAcceptableFrameRateFPS=30
MaxNetworkBandwidthMbps=100

; Log verbosity (Debug, Info, Warning)
LogLevel=Warning
```

---

## Bot Behavior Types

### Realistic
- Moves between waypoints at variable speed
- Occasional pauses/stops
- Engages enemies if applicable
- Sends frequent position updates
- Most CPU intensive

**Use for:** Simulating actual player behavior, finding realistic bottlenecks

```cpp
UStargateStress::SpawnBotsForLoadTest(50, TEXT("Realistic"));
```

### Aggressive
- Actively seeks combat/engagement
- High network traffic (frequent updates)
- Complex AI decision-making
- Movement-heavy

**Use for:** PvP stress tests, combat system validation

```cpp
UStargateStress::SpawnBotsForLoadTest(50, TEXT("Aggressive"));
```

### Passive
- Stationary or minimal movement
- Low network traffic
- Minimal CPU usage
- Good for connection limit testing

**Use for:** Testing server capacity (sheer connection count), CCU limits

```cpp
UStargateStress::SpawnBotsForLoadTest(200, TEXT("Passive"));
```

---

## Performance Optimization Tips

### Reduce Bot Complexity
Use "Passive" bots for initial load testing:
```cpp
UStargateStress::SpawnBotsForLoadTest(100, TEXT("Passive"));  // Fast
// vs
UStargateStress::SpawnBotsForLoadTest(30, TEXT("Realistic")); // Slow but realistic
```

### Distribute Across Maps
Spread bots across multiple maps to avoid rendering all simultaneously:
```cpp
UStargateStress::SpawnBotsForLoadTest(50);  // Map A
LoadAndRamupMaps({MapB, MapC});            // Spawn bots on other maps
```

### Monitor Selectively
Don't record every metric — focus on key ones:
```cpp
// Focus on frame time and memory only
FStressMetrics Metrics = UStargateStress::GetCurrentMetrics();
// Skip: CPU usage, network bandwidth for faster collection
```

---

## Troubleshooting

### Bots Not Moving

**Problem:** Spawned bots stand still

**Solutions:**
1. Verify map has waypoints/patrol points defined
2. Check `bPatrolWaypoints = true` in behavior config
3. Ensure bots have valid AI controller
4. Increase bot count — few bots may idle randomly

### Memory Grows Unbounded

**Problem:** Memory usage climbs continuously

**Solutions:**
1. Check for memory leaks in bot cleanup (`CleanupAllBots()`)
2. Verify destroyed actors aren't being kept in arrays
3. Monitor bot component lifecycle
4. Use memory profiler to find leak source

### Performance Degrades Unevenly

**Problem:** Frame rate degrades worse than expected with player count

**Solutions:**
1. Switch to "Passive" bots to isolate rendering cost
2. Check if AI pathfinding is cache-busting
3. Verify LOD systems are working
4. Profile CPU time per bot type

---

## See Also

- [ArgusLens Performance Monitoring](./ARGUSLENS_GUIDE.md)
- [FringeNetwork Chaos Testing](./FRINGENETWORK_GUIDE.md)
- [Nexus Core Framework](./NEXUS_GUIDE.md)
