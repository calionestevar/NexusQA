# Integration Guide — Using Multiple NexusQA Modules Together

## Overview

NexusQA provides 8 specialized modules designed to work together. This guide shows how to combine them effectively in your test suite, avoiding common pitfalls discovered during real-world integration.

---

## When to Use NEXUS_TEST vs NEXUS_TEST_GAMETHREAD

### NEXUS_TEST (Parallel Execution - Default)

**Use for:** Pure logic tests, utilities, API validation, calculations

**Characteristics:**
- Executes on thread pool (multiple tests in parallel)
- ⚡ Fast execution
- Safe to call any const/pure functions
- **NO actor spawning**
- **NO world interaction**
- **NO GWorld access**

**Example:**
```cpp
NEXUS_TEST(FComplexMathTest, "Complex Math Validation", ETestPriority::Normal)
{
    // Pure logic - safe for parallel execution
    float Result = ComplexCalculation(3.14f, 2.71f);
    return FMath::Abs(Result - 5.85f) < 0.01f;
}
```

### NEXUS_TEST_GAMETHREAD (Sequential Execution - Game Thread Only)

**Use for:** Anything requiring game world, actors, or world context

**Characteristics:**
- Executes on main game thread
- Sequential (one test at a time)
- 🔒 Safe for actor/object creation
- Required for `GWorld->SpawnActor<>()`
- Required for `NewObject<>()`
- Must check `IsInAsyncLoadingThread()` before running

**Example:**
```cpp
NEXUS_TEST_GAMETHREAD(FActorSpawningTest, "Actor Spawning Test", ETestPriority::Normal)
{
    // Safe to spawn actors on game thread
    ACharacter* TestCharacter = GWorld->SpawnActor<ACharacter>(
        ACharacter::StaticClass(),
        FVector(0, 0, 0),
        FRotator::ZeroRotator
    );
    
    bool bSuccess = TestCharacter != nullptr;
    TestCharacter->Destroy();
    return bSuccess;
}
```

### Decision Tree

```
Does your test need to:
├─ Spawn actors?                     → NEXUS_TEST_GAMETHREAD
├─ Create UObjects with NewObject?  → NEXUS_TEST_GAMETHREAD
├─ Access GWorld?                   → NEXUS_TEST_GAMETHREAD
├─ Call game callbacks/delegates?   → NEXUS_TEST_GAMETHREAD
├─ Test rendering/animation?        → NEXUS_TEST_GAMETHREAD
└─ Pure logic/API validation?       → NEXUS_TEST (default)
```

---

## Common Integration Patterns

### Pattern 1: Compliance + Performance Testing

**Goal:** Validate game meets compliance requirements while maintaining performance

**Module Combination:**
- **Protego** — COPPA/GDPR/DSA compliance checks (pure logic)
- **ArgusLens** — Performance monitoring and gates
- **ObserverNetworkDashboard** — Live event dashboard

**Implementation:**
```cpp
NEXUS_TEST(FCompleteComplianceTest, "Full Compliance Validation", ETestPriority::Smoke)
{
    // Start performance monitoring
    UArgusLens* Lens = NewObject<UArgusLens>();
    Lens->StartPerformanceMonitoring();
    
    // Run compliance checks
    UProtego* Compliance = NewObject<UProtego>();
    bool bCOPPAValid = Compliance->ValidateCOPPA();
    bool bGDPRValid = Compliance->ValidateGDPR();
    bool bDSAValid = Compliance->ValidateDSA();
    
    // Check performance gates
    Lens->StopPerformanceMonitoring();
    bool bPerformancePassed = Lens->DidPassPerformanceGates();
    
    // Log to dashboard
    if (bCOPPAValid && bGDPRValid && bDSAValid && bPerformancePassed)
    {
        PALANTIR_BREADCRUMB(TEXT("Compliance"), TEXT("✅ All checks passed"));
        return true;
    }
    
    PALANTIR_BREADCRUMB(TEXT("Compliance"), TEXT("❌ Some checks failed"));
    return false;
}
```

### Pattern 2: Network Testing + Game Systems

**Goal:** Test network behavior with realistic game-world impact

**Module Combination:**
- **FringeNetwork** — HTTP testing, lag injection, chaos engineering
- **Nexus** — Game-thread tests for world interaction
- **Palantir** — Distributed trace IDs for correlated logging

**Implementation:**
```cpp
NEXUS_TEST_GAMETHREAD(FNetworkGameplayTest, "Network Impact on Gameplay", ETestPriority::Critical)
{
    FPalantirTraceGuard TraceGuard;  // Auto trace context
    
    // Create test character
    ACharacter* TestChar = GWorld->SpawnActor<ACharacter>();
    
    // Inject network chaos
    UFringeNetwork* FringeNet = NewObject<UFringeNetwork>();
    FringeNet->InjectLatency(100.0f);  // 100ms lag
    
    // Test game behavior under network stress
    FVector InitialPos = TestChar->GetActorLocation();
    TestChar->AddMovementInput(FVector::ForwardVector, 1.0f);
    
    // Verify movement still works despite lag
    FVector FinalPos = TestChar->GetActorLocation();
    bool bMovedDespiteLag = !FinalPos.Equals(InitialPos);
    
    // Cleanup
    FringeNet->RestoreNetwork();
    TestChar->Destroy();
    
    return bMovedDespiteLag;
}
```

### Pattern 3: Load Testing + Monitoring

**Goal:** Verify game stability under concurrent load

**Module Combination:**
- **StargateStress** — Bot simulation and concurrent user spawning
- **ArgusLens** — Real-time FPS and memory monitoring
- **ObserverNetworkDashboard** — Live metrics visualization

**Implementation:**
```cpp
NEXUS_TEST_GAMETHREAD(FStressTestWithMonitoring, "Load Test With Monitoring", ETestPriority::Normal)
{
    UArgusLens* Monitor = NewObject<UArgusLens>();
    Monitor->StartPerformanceMonitoring();
    
    UStargateStress* Stress = NewObject<UStargateStress>();
    Stress->SpawnBots(50);  // Spawn 50 concurrent bots
    
    // Let game run under load
    FPlatformProcess::Sleep(5.0f);
    
    // Check performance didn't degrade below thresholds
    Stress->DespawnBots();
    Monitor->StopPerformanceMonitoring();
    
    return Monitor->DidPassPerformanceGates();
}
```

### Pattern 4: API Testing with Distributed Tracing

**Goal:** Test REST/GraphQL APIs while maintaining request traceability

**Module Combination:**
- **Palantir** — Fluent API testing with automatic trace ID injection
- **FringeNetwork** — Network chaos injection during API tests
- **ObserverNetworkDashboard** — API request logging

**Implementation:**
```cpp
NEXUS_TEST(FGameAPITest, "GameServer API Validation", ETestPriority::Smoke)
{
    FPalantirTraceGuard TraceGuard;  // Auto trace context
    
    // Make API call with fluent interface
    FPalantirResponse Response = PalantirRequest()
        .SetURL("https://api.example.com/players")
        .SetMethod(EHttpMethod::GET)
        .SetTimeout(5.0f)
        .ExpectStatus(200)
        .ExpectJsonPath("$.players[0].name", "TestPlayer")
        .Send();
    
    // Trace ID automatically flows through request
    return Response.StatusCode == 200;
}
```

---

## Initialization Order (Critical!)

Tests must initialize modules in the correct order for proper operation:

```cpp
void InitializeNexusQA()
{
    // STEP 1: Load compliance rules (done once at startup)
    UProtego* Compliance = NewObject<UProtego>();
    Compliance->LoadComplianceRules("Content/Compliance/");
    
    // STEP 2: Initialize observer dashboard (required for event logging)
    UObserverNetworkDashboard::Initialize();
    
    // STEP 3: Configure performance thresholds
    UArgusLens* Lens = NewObject<UArgusLens>();
    Lens->SetFPSThreshold(60.0f);
    Lens->SetMemoryThreshold(2048);  // 2GB
    
    // STEP 4: Discover tests (NEXUS_TEST and NEXUS_TEST_GAMETHREAD)
    UNexusCore::DiscoverAllTests();
    
    // STEP 5: Run tests
    UNexusCore::RunAllTests(true);  // true = parallel execution
}
```

**Why this order matters:**
1. Compliance rules must load before Protego tests
2. Dashboard initialization registers event listeners
3. Performance thresholds affect ArgusLens validation
4. Test discovery populates the test list
5. RunAllTests executes in correct sequence (parallel → game-thread)

---

## Module Interaction Map

```
Nexus (Orchestrator)
├── NEXUS_TEST macros (parallel execution)
│   ├─→ Protego (pure compliance checks)
│   ├─→ ArgusLens (performance monitoring)
│   └─→ Palantir (API testing)
│
└── NEXUS_TEST_GAMETHREAD macros (sequential execution)
    ├─→ FringeNetwork (network chaos, world impact)
    ├─→ StargateStress (bot spawning, load)
    └─→ ArgusLens (gameplay performance)

Palantir (Tracing Hub)
├─→ Automatically injects trace IDs
├─→ Logs to ObserverNetworkDashboard
└─→ Propagates through all module calls

ObserverNetworkDashboard (Monitoring)
├─→ Receives events from all modules
├─→ Real-time ImGui dashboard
└─→ HTML report generation
```

---

## Known Limitations & Workarounds

### Limitation 1: Async HTTP Race Conditions

**Problem:**
HTTP request callbacks execute on HTTP thread. If main thread returns the completion event to the pool before callback finishes, you get use-after-free crashes.

**Workaround:**
Palantir's `ExecuteBlocking()` handles this internally. Use it instead of raw HTTP requests:

```cpp
// ❌ DON'T: Raw HTTP request (unsafe)
Request->ProcessRequest();  // May crash if callback late

// ✅ DO: Use Palantir's blocking API (safe)
FPalantirRequest Request;
FPalantirResponse Response = Request.ExecuteBlocking();
```

### Limitation 2: ImGui Dashboard Requires Game Thread

**Problem:**
ImGui rendering must happen on game thread. If you call dashboard methods from async tests, you'll hit assertions.

**Workaround:**
Log events to dashboard via PALANTIR_BREADCRUMB (thread-safe), not direct ImGui calls:

```cpp
// ❌ DON'T: Direct ImGui from async context
ImGui::Text("Test passed");  // Assertion on non-game thread

// ✅ DO: Use PALANTIR_BREADCRUMB (thread-safe)
PALANTIR_BREADCRUMB(TEXT("TestEvent"), TEXT("Test passed"));
```

### Limitation 3: Actor Spawning Requires Valid GWorld

**Problem:**
`GWorld->SpawnActor<>()` crashes if:
- Game is shutting down
- Asset loading is in progress (`IsInAsyncLoadingThread()` is true)
- World is being unloaded

**Workaround:**
Always check world validity before spawning in game-thread tests:

```cpp
NEXUS_TEST_GAMETHREAD(FActorTest, "Actor Spawning", ETestPriority::Normal)
{
    // Check world is valid
    if (!GWorld || GWorld->bIsTearingDown)
    {
        return false;
    }
    
    // Check not in async loading
    if (IsInAsyncLoadingThread())
    {
        return false;
    }
    
    ACharacter* TestChar = GWorld->SpawnActor<ACharacter>();
    bool bSuccess = TestChar != nullptr;
    if (TestChar) TestChar->Destroy();
    return bSuccess;
}
```

### Limitation 4: NewObject<> Creates Uninitialized Objects

**Problem:**
`NewObject<UMyClass>()` creates objects without calling BeginPlay() or initializing game logic.

**Workaround:**
Manually initialize created objects or use factory functions:

```cpp
// ❌ INCOMPLETE: NewObject without init
UCharacterComponent* Comp = NewObject<UCharacterComponent>();
// Component won't be attached, properties uninitialized

// ✅ CORRECT: Manual initialization
UCharacterComponent* Comp = NewObject<UCharacterComponent>();
Comp->RegisterComponent();
Comp->BeginPlay();  // Manual init if needed
```

### Limitation 5: Tests Cannot Run During Asset Streaming

**Problem:**
Many UE systems freeze during asset loading. Tests that access game state will hang or crash.

**Workaround:**
Check `IsInAsyncLoadingThread()` or use `WaitForAsyncLoading()`:

```cpp
NEXUS_TEST_GAMETHREAD(FAssetTest, "Asset Loading Test", ETestPriority::Normal)
{
    // Wait for any pending asset loads
    if (!GAsyncLoadingTask.IsComplete())
    {
        FStreamingManager::Get().BlockTillAllRequestsFinished();
    }
    
    // Now safe to access game state
    return GWorld != nullptr;
}
```

---

## Troubleshooting Integration Issues

### Issue: "Unhandled Exception: EXCEPTION_ACCESS_VIOLATION reading address 0xffffffffffffffff"

**Likely cause:** HTTP callback use-after-free

**Solution:**
- Use `PalantirRequest().ExecuteBlocking()` instead of raw HTTP
- Ensure all captures in HTTP callbacks are shared pointers
- Check that completion events aren't returned to pool early

### Issue: "IsInAsyncLoadingThread assertion failed"

**Likely cause:** Game-thread code in wrong macro

**Solution:**
- Use `NEXUS_TEST_GAMETHREAD` for anything accessing GWorld
- Move to `NEXUS_TEST` if logic is pure/doesn't need world
- Check `IsInAsyncLoadingThread()` before spawning actors

### Issue: "Fatal error: Attempted to use a None class default object"

**Likely cause:** NewObject<> without proper initialization

**Solution:**
- Call `RegisterComponent()` on components
- Call `BeginPlay()` manually if needed
- Use factory functions instead of NewObject when available

### Issue: Dashboard shows no events

**Likely cause:** Events not being logged

**Solution:**
- Use `PALANTIR_BREADCRUMB()` in your tests
- Ensure ObserverNetworkDashboard::Initialize() was called
- Check that tests are actually running (check console output)

---

## Best Practices

### ✅ DO

- Use `NEXUS_TEST` for pure logic
- Use `NEXUS_TEST_GAMETHREAD` for world interaction
- Call modules in recommended initialization order
- Use `PALANTIR_BREADCRUMB()` for logging
- Check `GWorld != nullptr` before spawning actors
- Use shared pointers in async callbacks
- Clean up spawned actors with `Destroy()`

### ❌ DON'T

- Mix pure logic with world access in NEXUS_TEST
- Capture raw pointers in async lambdas
- Call ImGui directly (use PALANTIR_BREADCRUMB)
- Return events to pool before callbacks complete
- Spawn actors in NEXUS_TEST (wrong thread)
- Access GWorld in parallel tests
- Assume objects are initialized after NewObject()

---

## Example: Complete Integration Test Suite

```cpp
#include "Nexus/Public/NexusTest.h"
#include "Protego/Public/Protego.h"
#include "ArgusLens/Public/ArgusLens.h"
#include "FringeNetwork/Public/FringeNetwork.h"
#include "ObserverNetworkDashboard/Public/ObserverNetworkDashboard.h"

// Pure compliance test (parallel-safe)
NEXUS_TEST(FComplianceTest, "GDPR Compliance", ETestPriority::Smoke)
{
    UProtego* Compliance = NewObject<UProtego>();
    return Compliance->ValidateGDPR();
}

// Performance test (parallel-safe)
NEXUS_TEST(FPerformanceTest, "FPS Threshold", ETestPriority::Normal)
{
    UArgusLens* Lens = NewObject<UArgusLens>();
    Lens->StartPerformanceMonitoring();
    FPlatformProcess::Sleep(1.0f);
    Lens->StopPerformanceMonitoring();
    return Lens->DidPassPerformanceGates();
}

// Game-world test (sequential, game-thread only)
NEXUS_TEST_GAMETHREAD(FGameplayTest, "Actor Spawning", ETestPriority::Normal)
{
    if (!GWorld || GWorld->bIsTearingDown) return false;
    if (IsInAsyncLoadingThread()) return false;
    
    ACharacter* TestChar = GWorld->SpawnActor<ACharacter>();
    bool bSuccess = TestChar != nullptr;
    if (TestChar) TestChar->Destroy();
    
    PALANTIR_BREADCRUMB(TEXT("GameplayTest"), 
        bSuccess ? TEXT("✅ Passed") : TEXT("❌ Failed"));
    
    return bSuccess;
}

// Network test with game impact (sequential, game-thread)
NEXUS_TEST_GAMETHREAD(FNetworkStressTest, "Network Under Load", ETestPriority::Critical)
{
    FPalantirTraceGuard TraceGuard;
    
    UFringeNetwork* Network = NewObject<UFringeNetwork>();
    Network->InjectLatency(200.0f);
    
    ACharacter* TestChar = GWorld->SpawnActor<ACharacter>();
    FVector StartPos = TestChar->GetActorLocation();
    
    TestChar->AddMovementInput(FVector::ForwardVector, 1.0f);
    FVector EndPos = TestChar->GetActorLocation();
    
    bool bMovedDespiteLag = !EndPos.Equals(StartPos);
    
    Network->RestoreNetwork();
    TestChar->Destroy();
    
    return bMovedDespiteLag;
}
```

This integration suite runs:
1. **2 parallel tests first** (Compliance, Performance) - fast execution
2. **2 sequential game-thread tests** (Gameplay, Network) - safe world access
3. All with automatic tracing and dashboard logging

---

## Further Reading

- **[NEXUS_GUIDE.md](NEXUS_GUIDE.md)** — Test discovery and execution
- **[PROTEGO_GUIDE.md](PROTEGO_GUIDE.md)** — Compliance validation
- **[ARGUSLENS_GUIDE.md](ARGUSLENS_GUIDE.md)** — Performance monitoring
- **[FRINGENETWORK_GUIDE.md](FRINGENETWORK_GUIDE.md)** — Network chaos and testing
- **[API_TESTING.md](API_TESTING.md)** — REST/GraphQL API patterns
- **[PALANTIR.md](PALANTIR.md)** — Distributed tracing deep-dive
- **[OBSERVER_NETWORK_GUIDE.md](OBSERVER_NETWORK_GUIDE.md)** — Dashboard and event logging
