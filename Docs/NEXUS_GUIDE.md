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

**Example (Without Tags):**

```cpp
// These tests get "Untagged" automatically
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

### Custom Tags (Dynamic Test Categorization)

Tests can be tagged with arbitrary string categories for flexible organization and filtering. Use the `_TAGGED` variant of macros when you want to assign custom tags.

**Two Macro Families:**

1. **Without Tags** (Backwards compatible, gets "Untagged" automatically):
   ```cpp
   NEXUS_TEST(TestClassName, "Test.Path.Name", ETestPriority::Normal)
   NEXUS_TEST_GAMETHREAD(TestClassName, "Test.Path.Name", ETestPriority::Normal)
   NEXUS_PERF_TEST(TestClassName, "Test.Path.Name", ETestPriority::Normal, 60.0f)
   ```

2. **With Tags** (Explicit tag specification):
   ```cpp
   NEXUS_TEST_TAGGED(TestClassName, "Test.Path.Name", ETestPriority::Normal, {"Tag1", "Tag2"})
   NEXUS_TEST_GAMETHREAD_TAGGED(TestClassName, "Test.Path.Name", ETestPriority::Normal, {"Tag1", "Tag2"})
   NEXUS_PERF_TEST_TAGGED(TestClassName, "Test.Path.Name", ETestPriority::Normal, 60.0f, {"Tag1", "Tag2"})
   ```

**Features:**
- **Simple, explicit syntax** — Use untagged macros for no tags, `_TAGGED` variants for custom tags
- **Backwards compatible** — Existing tests without tags still work
- **Unlimited custom tags** — Define any tags you need (no predefined list)
- **Dynamic reports** — HTML reports generate tag sections automatically based on tags used
- **Programmatic filtering** — Query tests by custom tags at runtime
- **Automatic "Untagged" label** — Untagged tests appear in their own report section

**Common Tag Conventions:**
````

```cpp
// Category tags
NEXUS_TEST_TAGGED(FNetworkTest, "Network.Connection", ETestPriority::Normal, {"Networking"})
NEXUS_TEST_TAGGED(FPerformanceTest, "Performance.CPU", ETestPriority::High, {"Performance"})
NEXUS_TEST_TAGGED(FGameplayTest, "Gameplay.Combat", ETestPriority::Normal, {"Gameplay"})

// Priority/Severity tags
NEXUS_TEST_TAGGED(FCriticalTest, "Auth.Login", ETestPriority::Critical, {"Critical", "MustPass"})
NEXUS_TEST_TAGGED(FComplexTest, "AI.Pathfinding", ETestPriority::High, {"P1"})

// Compliance tags
NEXUS_TEST_TAGGED(FCOPPATest, "Compliance.AgeGating", ETestPriority::Critical, {"Compliance", "COPPA"})
NEXUS_TEST_TAGGED(FGDPRTest, "Compliance.DataRetention", ETestPriority::Critical, {"Compliance", "GDPR"})

// Feature/System tags
NEXUS_TEST_TAGGED(FReplicationTest, "Multiplayer.Replication", ETestPriority::Normal, {"Networking", "Multiplayer"})
NEXUS_TEST_TAGGED(FUITest, "UI.MainMenu", ETestPriority::Normal, {"UI", "Integration"})

// Mixed tags
NEXUS_TEST_TAGGED(FStressTest, "Performance.MaxLoad", ETestPriority::High, {"Performance", "Stress", "P1"})

// Untagged tests (backwards compatible)
NEXUS_TEST(FSimpleTest, "Simple.Test", ETestPriority::Normal)  // Gets "Untagged" automatically
```

**Retrieving Tags at Runtime:**

```cpp
// Get all tests with a specific tag
TArray<FNexusTest*> ComplianceTests = UNexusCore::GetTestsWithCustomTag(TEXT("Compliance"));

// Count tests with a tag
int32 NetworkingCount = UNexusCore::CountTestsWithCustomTag(TEXT("Networking"));

// Get all unique tags across all tests
TArray<FString> AllTags = UNexusCore::GetAllCustomTags();
```

**HTML Report Integration:**

When tests run, the generated LCARS HTML report automatically creates:
- **Tag distribution cards** showing count for each tag (including "Untagged")
- **Grouped test sections** organized by tag
- **Pass percentages** for each tag category

Tags are collected dynamically from actual test results, so only tags used in your tests appear in the report. Tests without explicit tags get the "Untagged" category.

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

### Running Game-Thread Tests with Auto-PIE

Game-thread tests require an active game world to function properly. NEXUS provides automatic PIE (Play-in-Editor) launching to handle this:

#### Configuration

To enable Auto-PIE launch, add the following to your `DefaultGame.ini`:

```ini
[/Script/Nexus.NexusSettings]
TestMapPath=/Game/Maps/YourTestMap
```

Replace `/Game/Maps/YourTestMap` with the path to your test level.

#### How It Works

When you run tests with the `Nexus.RunTests` console command:

1. **Detection Phase**: Framework checks if a PIE/game world is already active
2. **Auto-Launch Phase**: If no world found, framework automatically launches PIE with the configured `TestMapPath`
3. **Wait Phase**: Framework waits up to 5 seconds for the world to fully initialize
4. **Execution Phase**: Once world is ready, game-thread tests execute normally

#### Manual PIE Launch

If you prefer to launch PIE manually before running tests:

1. Click the "Play" button in the Unreal Editor toolbar
2. Wait for the game world to fully load
3. Run the `Nexus.RunTests` console command
4. The framework will detect the active world and proceed immediately

#### Graceful Fallback

If Auto-PIE fails or no `TestMapPath` is configured, game-thread tests gracefully skip:

```
⚠️  No active game world detected
💡 Tip: Click 'Play' in the editor to start PIE mode before running tests
```

This is expected behavior and prevents test failures due to missing world context.

---

## Advanced Features

### Test Timeout Handling

Prevent tests from running indefinitely by setting maximum duration:

```cpp
NEXUS_TEST(FLongRunningTest, "Performance.Timeout.LongOp", ETestPriority::Normal)
{
    // Configure 30-second timeout
    // (in test class constructor or via test setup)
    // MaxDurationSeconds = 30.0;
    
    // If this takes > 30 seconds, test auto-fails with timeout error
    SimulateExpensiveOperation();
    return true;
}
```

Timeouts are reported in logs with actual vs allowed duration for debugging.

### Test Fixtures (Setup/Teardown)

Share test data and perform setup/cleanup with `BeforeEach` and `AfterEach`:

```cpp
NEXUS_TEST(FFixtureExample, "Gameplay.Fixture.SharedState", ETestPriority::Normal)
{
    // BeforeEach runs before the test (and before each retry)
    // Perfect for spawning actors, initializing state
    BeforeEach = [this](FNexusTestContext& Context) -> bool {
        if (!Context.IsValid()) return false;
        // Setup: Create test character
        Context.SpawnTestCharacter();
        return true;
    };
    
    // AfterEach runs after the test, even if it fails
    // Perfect for cleanup
    AfterEach = [this](FNexusTestContext& Context) {
        // Teardown: Automatic via context RAII
        // But you can do custom cleanup here
        Context.CleanupSpawnedActors();
    };
    
    return true;
}
```

### Test Filtering by Tags

Run specific test subsets using tags (useful for CI/CD workflows):

```cpp
// Tag your tests
NEXUS_TEST(FNetworkTest, "Network.Multiplayer.Sync", ETestPriority::Normal)
{
    // In test class, set tags:
    // Tags = ETestTag::Networking | ETestTag::Integration;
    return true;
}

// Run only networking tests
UNexusCore::RunTestsWithTags(ETestTag::Networking);

// Available tags:
// - Networking: Network/multiplayer tests
// - Performance: Benchmark/perf tests
// - Gameplay: Mechanics tests
// - Compliance: Regulation tests (COPPA/GDPR/DSA)
// - Integration: Multi-module tests
// - Stress: Load/stress tests
// - Editor: Editor-only tests
// - Rendering: Graphics tests
```

### Automatic Test Retry with Exponential Backoff

Flaky network tests automatically retry on failure:

```cpp
NEXUS_TEST(FFlakeyNetworkTest, "Network.Flaky.Retry", ETestPriority::Normal)
{
    // Configure 3 retries
    // MaxRetries = 3;  // Will retry up to 3 times: 1s, 2s, 4s delays
    
    // If test fails, retry with exponential backoff
    // 1st attempt fails → wait 1s → 2nd attempt
    // 2nd attempt fails → wait 2s → 3rd attempt
    // 3rd attempt fails → wait 4s → 4th attempt
    return TestFlakeyNetwork();  // If any attempt passes, test passes
}
```

### Test Result History & Trend Analysis

Detect performance regressions automatically:

```cpp
// After running tests, analyze trends
int32 Regressions = UNexusCore::DetectRegressions();

if (Regressions > 0)
{
    UE_LOG(LogTemp, Warning, TEXT("⚠️ Performance regression detected: %d tests slower than baseline"), Regressions);
}

// Export detailed trend reports
UNexusCore::ExportTestTrends(TEXT("Saved/TestTrends/"));

// Get statistics
double AvgDuration = UNexusCore::GetAverageTestDuration();
double MedianDuration = UNexusCore::GetMedianTestDuration();

UE_LOG(LogTemp, Display, TEXT("Average test duration: %.2fms"), AvgDuration * 1000);
UE_LOG(LogTemp, Display, TEXT("Median test duration: %.2fms"), MedianDuration * 1000);
```

**Outputs:**
- `test_trends.csv` — Frame-by-frame history (TestName, Timestamp, Duration, Passed)
- `test_trends_summary.json` — Summary with pass rates and average durations

### Skipping Tests Conditionally

Some tests need to be skipped based on runtime conditions (e.g., network unavailable, feature disabled). Use `NEXUS_SKIP_TEST()` to mark tests as skipped instead of failing:

```cpp
#include "Nexus/Core/Public/NexusCore.h"
#include "ISocketSubsystem.h"

NEXUS_TEST(FNetworkTest_OnlineOnly, "Network.Remote.API", ETestPriority::OnlineOnly)
{
    // Check if network is available before running
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!SocketSubsystem) {
        NEXUS_SKIP_TEST("Network unavailable - skipping online test");
    }
    
    // If we get here, network is available
    bool bSuccess = TestRemoteAPI();
    return bSuccess;
}

// Another example: feature-gated test
NEXUS_TEST(FExperimentalFeatureTest, "Game.Experimental.NewWeapon", ETestPriority::Normal)
{
    if (!IsExperimentalFeatureEnabled()) {
        NEXUS_SKIP_TEST("Experimental feature disabled in config");
    }
    
    // Test the experimental feature
    return TestWeaponSystem();
}

// Platform-specific test
NEXUS_TEST(FPlatformSpecificTest, "Platform.Mobile.Touch", ETestPriority::Normal)
{
#if !WITH_EDITOR || PLATFORM_ANDROID || PLATFORM_IOS
    // Test touch input
    return TestTouchInput();
#else
    NEXUS_SKIP_TEST("Touch input not supported on this platform");
#endif
}
```

**Key Points:**
- Skipped tests don't count as pass or fail
- Reported separately in test results and dashboard
- Shows skip reason in logs and reports
- Useful for tests that can't run in all environments
- Reports show: Passed / Failed / Skipped / Total

**In Reports/Dashboard:**
- ✓ Passed tests (green)
- ⚠️ Failed tests (red)
- ⏭️ Skipped tests (gold)
- Shows why tests were skipped in artifacts

### Stack Traces on Failure

Failed tests automatically capture diagnostic stack traces:

```cpp
// When a test fails, a stack trace is captured for debugging
// Access via: FNexusTest::LastResult.StackTrace
// Or export to reports automatically

for (const FNexusTestResult& Result : FNexusTest::AllResults)
{
    if (!Result.bPassed && Result.HasStackTrace())
    {
        UE_LOG(LogTemp, Error, TEXT("Test failure stack trace:\n%s"), 
            *Result.GetStackTraceString());
    }
}
```

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
