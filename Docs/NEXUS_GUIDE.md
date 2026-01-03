# Nexus — Core Testing Framework

**Nexus** is the heart of the NexusQA framework, providing test discovery, parallel execution, result aggregation, and console integration. It orchestrates all testing functionality and exposes tests through a simple, discoverable API.

> **New to NexusQA?** Start with [INTEGRATION_GUIDE.md](INTEGRATION_GUIDE.md) for multi-module patterns and best practices. This guide covers Nexus specifically.

---

## Overview

### Core Capabilities

| Feature | Purpose | Use Case |
|---------|---------|----------|
| **Test Discovery** | Auto-find all `NEXUS_TEST` macros | Build test suite at runtime, no registration needed |
| **Parallel Execution** | Run tests simultaneously | Execute 100s of tests in seconds using thread pool |
| **Fail-Fast Execution** | Stop on critical test failure | Prevent cascading failures, save test time |
| **Result Aggregation** | Collect all test results | Track pass/fail, duration, error messages |
| **Console Commands** | Run tests via `Nexus.RunTests` | Execute from editor console with one command |
| **LCARS Integration** | Export results to reports | Generate HTML/JSON/XML test reports |

---

## Quick Start

### 1. Add Nexus to Your Project

**In your `.Build.cs` file:**

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Nexus",
    "Json",
    "JsonUtilities"
});
```

### 2. Create Your First Test

**In any `.cpp` file (e.g., `MyTests.cpp`):**

```cpp
#include "Nexus/Core/Public/NexusCore.h"

// Simple test
NEXUS_TEST(FMyFirstTest, "MyGame.Gameplay.PlayerMovement", ETestPriority::Normal)
{
    // Test code here
    float MovementSpeed = 600.0f;
    
    if (MovementSpeed < 500.0f) {
        return false;  // Test failed
    }
    
    return true;  // Test passed
}

// Critical test (stops execution on failure)
NEXUS_TEST(FCriticalNetworkTest, "MyGame.Network.Connection", ETestPriority::Critical)
{
    // Test network connectivity
    bool bConnected = TestServerConnection();
    return bConnected;
}
```

### 3. Run Tests

**Option A: Console Command (Easiest)**

1. Start your game in editor
2. Press `~` to open console
3. Type: `Nexus.RunTests`
4. Tests execute and report generates automatically

**Option B: Programmatic**

```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Discover tests
    UNexusCore::DiscoverAllTests();
    
    // Run them
    UNexusCore::RunAllTests(true);  // true = parallel execution
    
    // Check results
    UE_LOG(LogTemp, Display, TEXT("Tests: %d/%d passed"),
        UNexusCore::PassedTests, UNexusCore::TotalTests);
}
```

---

## API Reference

### Test Macro

```cpp
NEXUS_TEST(TestClassName, "Test.Path.Name", ETestPriority::Normal)
{
    // Test implementation
}
```

Creates a test function that Nexus auto-discovers.

**Parameters:**

- `TestClassName` — Unique class name for test
- `"Test.Path.Name"` — Hierarchical test identifier (shows in reports)
- `ETestPriority::Normal` — Priority level (see below)

**Return Value:**
- `true` — Test passed
- `false` — Test failed

**Supported Priority Levels:**

```cpp
enum class ETestPriority : uint8
{
    Critical,  // Fail-fast: stops execution on failure
    High,      // Runs early, but doesn't stop on failure
    Normal,    // Regular priority
    Low,       // Runs last, lowest priority
};
```

**Example:**

```cpp
NEXUS_TEST(FBasicTest, "MyGame.Basic.Startup", ETestPriority::Critical)
{
    return GEngine != nullptr;  // True if engine initialized
}

NEXUS_TEST(FMovementTest, "MyGame.Movement.Speed", ETestPriority::Normal)
{
    ACharacter* TestChar = GetWorld()->SpawnActor<ACharacter>();
    float Speed = TestChar->GetCharacterMovement()->MaxWalkSpeed;
    return Speed > 500.0f;
}
```

---

### Discover All Tests

```cpp
void UNexusCore::DiscoverAllTests();
```

Scans all loaded modules for `NEXUS_TEST` macros and registers them.

**Called automatically:**
- On module startup (FNexusModule)
- Before running tests
- Can be called manually anytime

**Example:**

```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UNexusCore::DiscoverAllTests();
    
    UE_LOG(LogTemp, Display, TEXT("Discovered %d tests"), UNexusCore::TotalTests);
}
```

**What it does:**
- Reflects all `NEXUS_TEST` functions in memory
- Registers them in `DiscoveredTests` array
- Sorts by priority (Critical first)
- Updates `TotalTests` count

---

### Run All Tests

```cpp
void UNexusCore::RunAllTests(bool bParallel = false);
```

Executes all discovered tests sequentially or in parallel.

**Parameters:**
- `bParallel` — If true, uses thread pool for faster execution. If false, runs on game thread.

**Example:**

```cpp
// Sequential execution (safer, slower)
UNexusCore::RunAllTests(false);

// Parallel execution (faster, thread-safe)
UNexusCore::RunAllTests(true);
```

**What it does:**
- Sorts tests by priority
- Critical tests run first
- On critical test failure, stops execution (fail-fast)
- Records timing, result, and error message per test
- Updates `PassedTests` and `FailedTests` counters

---

### Get Test Results

```cpp
// Global counters
int32 UNexusCore::TotalTests;      // Total discovered tests
int32 UNexusCore::PassedTests;     // Tests that passed
int32 UNexusCore::FailedTests;     // Tests that failed
int32 UNexusCore::CriticalTests;   // Tests marked as Critical

// Discovered tests array
TArray<FNexusTest*> UNexusCore::DiscoveredTests;
```

**Example:**

```cpp
void CheckTestResults()
{
    UE_LOG(LogTemp, Display, TEXT("Test Summary:"));
    UE_LOG(LogTemp, Display, TEXT("  Total: %d"), UNexusCore::TotalTests);
    UE_LOG(LogTemp, Display, TEXT("  Passed: %d"), UNexusCore::PassedTests);
    UE_LOG(LogTemp, Display, TEXT("  Failed: %d"), UNexusCore::FailedTests);
    UE_LOG(LogTemp, Display, TEXT("  Critical: %d"), UNexusCore::CriticalTests);
    
    bool bAllPassed = (UNexusCore::FailedTests == 0);
    UE_LOG(LogTemp, Display, TEXT("  Status: %s"), 
        bAllPassed ? TEXT("✅ PASS") : TEXT("❌ FAIL"));
}
```

---

### Console Commands

#### Nexus.RunTests

Runs all discovered tests and generates report.

**Usage:**
```
~ (press to open console)
Nexus.RunTests
```

**What it does:**
- Discovers all tests
- Runs them in parallel
- Logs pass/fail count
- Generates LCARS report to `Saved/NexusReports/`
- Outputs report location to console

**Example output:**
```
🧪 NEXUS: Discovering tests...
🧪 NEXUS: Running 15 test(s)...
✅ NEXUS: Complete — 14/15 passed
📊 NEXUS: Report exported to Saved/NexusReports/
```

---

## Test Context and World Access

Game-thread tests (`NEXUS_TEST_GAMETHREAD`) automatically receive a `FNexusTestContext` containing access to the game world, game state, and player controller. This enables comprehensive gameplay testing.

### FNexusTestContext Structure

```cpp
struct FNexusTestContext
{
    // Game world access
    UWorld* World = nullptr;
    AGameStateBase* GameState = nullptr;
    APlayerController* PlayerController = nullptr;
    
    // Test utilities
    AActor* SpawnTestCharacter() const;
    bool IsValid() const;
    
    // Performance metrics (when using NEXUS_PERF_TEST)
    FTestPerformanceMetrics PerformanceMetrics;
    bool HAS_PERF_DATA() const;
    bool AssertAverageFPS(float MinFPS) const;
    bool AssertMaxMemory(float MaxMB) const;
    bool AssertMaxHitches(uint32 MaxCount, float ThresholdMs) const;
};
```

### Game-Thread Tests with Context

```cpp
#include "Nexus/Core/Public/NexusCore.h"

NEXUS_TEST_GAMETHREAD(FWorldAccessTest, "Gameplay.World.Access", ETestPriority::High)
{
    const FNexusTestContext& Context = /* auto-provided */;
    
    // Check context is valid (world is available)
    if (!Context.IsValid()) {
        UE_LOG(LogTemp, Warning, TEXT("No game world - test skipped"));
        return true;  // Gracefully skip
    }
    
    // Access game world
    if (!Context.World) {
        return false;
    }
    
    // Access game state
    if (!Context.GameState) {
        return false;
    }
    
    // Spawn a test character
    AActor* TestActor = Context.SpawnTestCharacter();
    if (!TestActor) {
        return false;
    }
    
    // Test character properties
    float Health = 100.0f;  // Example property
    
    // Automatic cleanup of spawned actors
    return Health > 0.0f;
}
```

### Spawning Test Actors

```cpp
NEXUS_TEST_GAMETHREAD(FSpawnTest, "Gameplay.Spawn.Multiple", ETestPriority::Normal)
{
    const FNexusTestContext& Context = /* auto-provided */;
    
    if (!Context.IsValid()) {
        return true;  // Skip if no world
    }
    
    // Spawn multiple test characters
    TArray<AActor*> TestActors;
    for (int32 i = 0; i < 10; ++i) {
        AActor* Actor = Context.SpawnTestCharacter();
        if (!Actor) {
            break;  // Reached spawn limit or other error
        }
        TestActors.Add(Actor);
    }
    
    bool bResult = TestActors.Num() == 10;
    
    // Automatic cleanup happens when context is destroyed
    return bResult;
}
```

### Testing Game State

```cpp
NEXUS_TEST_GAMETHREAD(FGameStateTest, "Gameplay.GameState.Access", ETestPriority::Normal)
{
    const FNexusTestContext& Context = /* auto-provided */;
    
    if (!Context.IsValid()) {
        return true;  // Skip
    }
    
    // Query game state
    if (!Context.GameState) {
        return false;
    }
    
    // Check game state properties
    uint32 PlayerCount = Context.GameState->PlayerArray.Num();
    float GameTime = Context.GameState->GetServerWorldTimeSeconds();
    
    return PlayerCount >= 0 && GameTime >= 0.0f;
}
```

### Auto-Detection: No World Available?

When running tests in headless/command-line mode (no PIE), game-thread tests gracefully skip:

```
⚠️  No active game world detected
💡 Tip: Click 'Play' in the editor to start PIE mode before running tests
```

This is expected behavior. World-dependent tests are skipped, and world-independent tests continue.

---

## Integration Examples

### Example 1: Test Suite for Game Features

```cpp
// GameplayTests.cpp
#include "Nexus/Core/Public/NexusCore.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

NEXUS_TEST(FCharacterSpawnTest, "Gameplay.Character.Spawn", ETestPriority::Critical)
{
    ACharacter* TestChar = GetWorld()->SpawnActor<ACharacter>();
    if (!TestChar) return false;
    
    TestChar->Destroy();
    return true;
}

NEXUS_TEST(FMovementSpeedTest, "Gameplay.Character.MovementSpeed", ETestPriority::Normal)
{
    ACharacter* TestChar = GetWorld()->SpawnActor<ACharacter>();
    float MaxSpeed = TestChar->GetCharacterMovement()->MaxWalkSpeed;
    
    TestChar->Destroy();
    return MaxSpeed >= 600.0f;
}

NEXUS_TEST(FJumpTest, "Gameplay.Character.Jump", ETestPriority::Normal)
{
    ACharacter* TestChar = GetWorld()->SpawnActor<ACharacter>();
    
    // Test jump
    TestChar->Jump();
    FPlatformProcess::Sleep(0.1f);
    
    bool bJumped = TestChar->GetCharacterMovement()->IsMovingVertically();
    TestChar->Destroy();
    
    return bJumped;
}

NEXUS_TEST(FInventoryTest, "Gameplay.Inventory.AddItem", ETestPriority::Normal)
{
    // Test inventory system
    FInventoryItem Item;
    Item.ID = 1;
    Item.Count = 10;
    
    // Would call real inventory API here
    return Item.Count > 0;
}
```

---

### Example 2: Critical vs. Non-Critical Tests

```cpp
// CriticalTests.cpp

// This test blocks others if it fails
NEXUS_TEST(FServerConnectionTest, "Network.Server.Connection", ETestPriority::Critical)
{
    return CheckServerConnection();  // If false, all remaining tests skipped
}

// These run after critical passes
NEXUS_TEST(FAuthenticationTest, "Network.Auth.Login", ETestPriority::High)
{
    return AuthenticateTestUser();
}

NEXUS_TEST(FGameplayTest, "Network.Gameplay.Sync", ETestPriority::Normal)
{
    return SyncGameplayState();
}
```

**Execution order:**
1. Critical tests (stops on first failure)
2. High priority tests
3. Normal priority tests
4. Low priority tests

---

### Example 3: Integration with Console Commands

```cpp
// TestGameMode.cpp
#include "TestGameMode.h"
#include "Nexus/Core/Public/NexusCore.h"

void ATestGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Console command is auto-registered by NexusModule
    // Users can type: Nexus.RunTests
    
    UE_LOG(LogTemp, Warning, TEXT("🧪 Test framework ready. Type 'Nexus.RunTests' in console"));
}
```

**User runs:**
```
~ 
Nexus.RunTests
```

**Output:**
```
🧪 NEXUS: Discovering tests...
🧪 NEXUS: Running 25 test(s)...
✅ NEXUS: Complete — 24/25 passed
📊 NEXUS: Report exported to Saved/NexusReports/LCARS_Report_2026-01-01_143045.html
```

---

## Writing Effective Tests

### ✅ Good Test Pattern

```cpp
NEXUS_TEST(FValidPlayerMovementTest, "Gameplay.Movement.Valid", ETestPriority::Normal)
{
    // 1. Setup: Create test environment
    ACharacter* TestChar = GetWorld()->SpawnActor<ACharacter>();
    if (!TestChar) return false;
    
    // 2. Execute: Perform the test
    TestChar->AddMovementInput(FVector::ForwardVector, 1.0f);
    FPlatformProcess::Sleep(0.1f);
    
    // 3. Assert: Check results
    FVector Pos = TestChar->GetActorLocation();
    bool bMoved = Pos.X > 0.0f;
    
    // 4. Cleanup: Free resources
    TestChar->Destroy();
    
    // 5. Return: Result
    return bMoved;
}
```

### ❌ Bad Test Pattern

```cpp
NEXUS_TEST(FBadTest, "Bad.Test", ETestPriority::Normal)
{
    // ❌ No setup
    // ❌ No cleanup (TestChar leaked)
    // ❌ Slow sleep without reason
    // ❌ No assertions, just return true
    
    ACharacter* TestChar = GetWorld()->SpawnActor<ACharacter>();
    FPlatformProcess::Sleep(5.0f);  // Why?
    return true;  // Always passes!
}
```

### Best Practices

**DO:**
- Keep tests focused (one thing per test)
- Clean up resources (destroy actors, free memory)
- Use realistic test data
- Make assertions clear and specific
- Handle async operations properly

**DON'T:**
- Create dependencies between tests
- Leave actors/memory lying around
- Use arbitrary sleep times
- Return true without checking anything
- Make tests slow (keep under 1 second each)

---

## Performance Tuning

### Parallel vs. Sequential

**Parallel (Recommended):**
```cpp
UNexusCore::RunAllTests(true);  // Uses thread pool
// 100 tests in ~2 seconds
```

**Sequential:**
```cpp
UNexusCore::RunAllTests(false);  // Game thread only
// 100 tests in ~20+ seconds
```

### Optimize Test Speed

1. **Avoid real server calls** — Mock networking
2. **Minimize actor spawning** — Reuse test objects
3. **Use GetWorld()->TimerManager()** — Don't use FPlatformProcess::Sleep()
4. **Batch related tests** — One setup, multiple assertions

---

## Troubleshooting

### Tests Not Discovered

**Problem:** `UNexusCore::TotalTests` is 0

**Solutions:**
1. Verify `NEXUS_TEST` macros are in `.cpp` files (not headers)
2. Ensure module is loaded before `DiscoverAllTests()`
3. Check test function signature matches: `[TestClass]Test([FNexusTest&])`
4. Call `DiscoverAllTests()` explicitly if not auto-discovered

### Tests Always Pass

**Problem:** Tests pass even when they should fail

**Solutions:**
1. Check test returns `false` on error
2. Verify assertions are correct
3. Look for missing cleanup code that masks failures
4. Add logging to see what's actually happening

### Tests Crash

**Problem:** Test execution crashes game

**Solutions:**
1. Add null checks for spawned actors
2. Use try-catch for risky operations
3. Reduce memory allocation (too many actors)
4. Check for invalid memory access
5. Run single test first to isolate crash

### Parallel Tests Fail But Sequential Pass

**Problem:** Race condition or thread-safety issue

**Solutions:**
1. Add synchronization (critical sections, mutexes)
2. Ensure no shared mutable state
3. Use thread-local storage for thread-specific data
4. Test separately with multiple runs

---

## Report Generation

After tests complete, LCARS generates reports:

**Files created in `Saved/NexusReports/`:**
- `LCARS_Report_<timestamp>.html` — Visual dashboard
- `nexus-results.xml` — JUnit XML for CI/CD
- `test_<name>.log` — Per-test logs
- `performance_<test>.json` — Timing metrics

**Example HTML report structure:**
```html
<body>
  <div class="test-summary">
    <h1>🖖 LCARS Report</h1>
    <p>Total Tests: 25</p>
    <p>Passed: 24 (96%)</p>
    <p>Failed: 1 (4%)</p>
  </div>
  
  <div class="test-results">
    <table>
      <tr><th>Test Name</th><th>Status</th><th>Duration</th></tr>
      <tr><td>Gameplay.Movement.Speed</td><td>✅ PASS</td><td>0.23s</td></tr>
      ...
    </table>
  </div>
</body>
```

---

## See Also

- [API Testing with PalantírRequest](./API_TESTING.md)
- [LCARS Reporting System](./LCARS_PROVIDERS.md)
- [Distributed Tracing with Palantír](./PALANTIR.md)
- [ArgusLens Performance Monitoring](./ARGUSLENS_GUIDE.md)
- [FringeNetwork Chaos Testing](./FRINGENETWORK_GUIDE.md)
- [StargateStress Load Testing](./STARGATESTRESS_GUIDE.md)
- [Protego Compliance Testing](./PROTEGO_GUIDE.md)
