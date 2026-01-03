# ArgusLens — Real-Time Performance Monitoring

**ArgusLens** (named after Argus Panoptes, the many-eyed giant) provides real-time performance monitoring, FPS tracking, memory profiling, and hitch detection. Use it to understand how your game performs under test conditions and validate performance gates.

---

## Overview

### Core Capabilities

| Feature | Purpose | Use Case |
|---------|---------|----------|
| **FPS Monitoring** | Track frame rate in real-time | Ensure 60 FPS target, detect frametime spikes |
| **Memory Profiling** | Monitor heap allocation | Catch memory leaks, validate memory budgets |
| **Hitch Detection** | Find frames exceeding threshold | Identify stutter sources |
| **Performance Gates** | Automated pass/fail checks | CI/CD validation: min FPS, max memory |
| **Artifact Export** | JSON/CSV/HTML reports | Shareable performance data |

---

## Quick Start

### 1. Add ArgusLens to Your Project

**In your `.Build.cs` file:**

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "ArgusLens",
    "Json",
    "JsonUtilities"
});
```

### 2. Start Monitoring

**In your game code:**

```cpp
#include "ArgusLens/Public/ArgusLens.h"

void MyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Start monitoring for 60 seconds
    UArgusLens::StartPerformanceMonitoring(60.0f);
}
```

### 3. Set Performance Gates

```cpp
void MyGameMode::SetPerformanceRequirements()
{
    FPerformanceThreshold Thresholds;
    Thresholds.MinFPS = 60.0f;
    Thresholds.MaxFrameTimeMs = 16.67f;  // 1000 / 60
    Thresholds.MaxMemoryMb = 2048.0f;
    Thresholds.HitchThresholdMs = 33.0f;  // 2 frames at 60 FPS
    
    UArgusLens::SetPerformanceThresholds(Thresholds);
}
```

### 4. Check Results

```cpp
void MyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    UArgusLens::StopPerformanceMonitoring();
    
    float AverageFPS = UArgusLens::GetAverageFPS();
    float PeakMemory = UArgusLens::GetPeakMemoryMb();
    int32 HitchCount = UArgusLens::GetHitchCount();
    bool bPassed = UArgusLens::DidPassPerformanceGates();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance: %.1f FPS, %.0f MB, %d hitches - %s"),
        AverageFPS, PeakMemory, HitchCount, bPassed ? TEXT("PASS") : TEXT("FAIL"));
    
    // Export report for analysis
    UArgusLens::ExportPerformanceArtifact(TEXT("Saved/PerformanceReports/"));
}
```

---

## API Reference

### Start Monitoring

```cpp
void UArgusLens::StartPerformanceMonitoring(
    float DurationSeconds = 60.0f,
    bool bTrackNetRelevancy = true
);
```

Begins performance sampling. Call once at test start.

**Parameters:**
- `DurationSeconds` — How long to monitor (default 60 seconds)
- `bTrackNetRelevancy` — Also track network-related metrics (default true)

**Example:**

```cpp
// Monitor for entire match (estimated 10 minutes)
UArgusLens::StartPerformanceMonitoring(600.0f);

// Monitor just a loading sequence (5 seconds)
UArgusLens::StartPerformanceMonitoring(5.0f);
```

**What it does:**
- Initializes sample buffer
- Records initial memory state
- Starts frame-by-frame sampling
- Can run indefinitely if `DurationSeconds` is long

---

### Stop Monitoring

```cpp
void UArgusLens::StopPerformanceMonitoring();
```

Ends monitoring and finalizes data. Call when test phase completes.

**Example:**

```cpp
void ATestGameMode::OnTestPhaseComplete()
{
    UArgusLens::StopPerformanceMonitoring();
    
    // Now safe to query results
    float AvgFPS = UArgusLens::GetAverageFPS();
    UE_LOG(LogTemp, Display, TEXT("Test completed. Average FPS: %.1f"), AvgFPS);
}
```

---

### Set Performance Thresholds

```cpp
void UArgusLens::SetPerformanceThresholds(
    const FPerformanceThreshold& Thresholds
);
```

Define pass/fail criteria for automated validation.

**Structure:**

```cpp
struct FPerformanceThreshold
{
    // Minimum acceptable frame rate
    float MinFPS = 30.0f;
    
    // Maximum acceptable frame time (ms)
    float MaxFrameTimeMs = 33.0f;  // ~30 FPS
    
    // Maximum memory budget (MB)
    float MaxMemoryMb = 2048.0f;
    
    // Hitch threshold (frames exceeding this are counted)
    float HitchThresholdMs = 100.0f;
};
```

**Example:**

```cpp
// High-performance desktop target
FPerformanceThreshold DesktopThresholds;
DesktopThresholds.MinFPS = 120.0f;
DesktopThresholds.MaxFrameTimeMs = 8.33f;
DesktopThresholds.MaxMemoryMb = 4096.0f;
DesktopThresholds.HitchThresholdMs = 16.7f;
UArgusLens::SetPerformanceThresholds(DesktopThresholds);

// Mobile target
FPerformanceThreshold MobileThresholds;
MobileThresholds.MinFPS = 30.0f;
MobileThresholds.MaxFrameTimeMs = 33.33f;
MobileThresholds.MaxMemoryMb = 512.0f;
MobileThresholds.HitchThresholdMs = 100.0f;
UArgusLens::SetPerformanceThresholds(MobileThresholds);
```

---

### Get Average FPS

```cpp
float UArgusLens::GetAverageFPS();
```

Returns the average FPS over the monitoring period.

**Example:**

```cpp
UArgusLens::StartPerformanceMonitoring(30.0f);
// ... gameplay for 30 seconds ...
UArgusLens::StopPerformanceMonitoring();

float AvgFPS = UArgusLens::GetAverageFPS();
if (AvgFPS < 60.0f) {
    UE_LOG(LogTemp, Error, TEXT("Performance target missed: %.1f FPS"), AvgFPS);
}
```

---

### Get Peak Memory

```cpp
float UArgusLens::GetPeakMemoryMb();
```

Returns the highest memory usage during monitoring.

**Example:**

```cpp
float PeakMem = UArgusLens::GetPeakMemoryMb();
UE_LOG(LogTemp, Display, TEXT("Peak memory: %.0f MB"), PeakMem);

if (PeakMem > 2048.0f) {
    UE_LOG(LogTemp, Error, TEXT("Memory budget exceeded!"));
}
```

---

### Get Hitch Count

```cpp
int32 UArgusLens::GetHitchCount();
```

Returns number of frames exceeding hitch threshold.

**Example:**

```cpp
int32 Hitches = UArgusLens::GetHitchCount();
if (Hitches > 5) {
    UE_LOG(LogTemp, Warning, TEXT("Detected %d frame hitches"), Hitches);
}
```

---

### Check Performance Gates

```cpp
bool UArgusLens::DidPassPerformanceGates();
```

Returns whether all metrics pass configured thresholds.

**Example:**

```cpp
bool bPassed = UArgusLens::DidPassPerformanceGates();
if (!bPassed) {
    UE_LOG(LogTemp, Error, TEXT("❌ Performance test FAILED"));
    return false;
}
UE_LOG(LogTemp, Display, TEXT("✅ Performance test PASSED"));
return true;
```

---

### Export Performance Report

```cpp
void UArgusLens::ExportPerformanceArtifact(
    const FString& OutputPath = TEXT("")
);
```

Generates JSON, CSV, and HTML reports for analysis.

**Example:**

```cpp
// Export to default location (Saved/PerformanceReports/)
UArgusLens::ExportPerformanceArtifact();

// Export to custom location
UArgusLens::ExportPerformanceArtifact(TEXT("Saved/MyReports/"));
```

**Output Files:**
- `performance_[timestamp].json` — Machine-readable metrics
- `performance_[timestamp].csv` — Frame-by-frame data
- `performance_[timestamp].html` — Visual dashboard

---

### Get Current Performance Snapshot

```cpp
FPerformanceSample UArgusLens::GetCurrentPerformanceSnapshot();
```

Captures current FPS, memory, and timing data. Useful for mid-test assertions.

**Structure:**

```cpp
struct FPerformanceSample
{
    float FrameTimeMs = 0.0f;      // Current frame time
    float FPS = 0.0f;              // Current FPS
    float MemoryMb = 0.0f;         // Current memory usage
    bool bIsHitch = false;         // Whether this frame is a hitch
    FString Timestamp;             // ISO 8601 timestamp
};
```

**Example:**

```cpp
FPerformanceSample Sample = UArgusLens::GetCurrentPerformanceSnapshot();

if (Sample.FPS < 30.0f) {
    UE_LOG(LogTemp, Warning, TEXT("⚠️ Frame rate dropped to %.1f FPS"), Sample.FPS);
}

if (Sample.bIsHitch) {
    UE_LOG(LogTemp, Warning, TEXT("🔴 Hitch detected: %.1fms"), Sample.FrameTimeMs);
}
```

---

## Integration Examples

### Example 1: Performance Gate Test

```cpp
// PerformanceGateTest.cpp
NEXUS_TEST(FPerformanceGateTest, "Performance.Desktop.60FPS", ETestPriority::Critical)
{
    // Configure thresholds
    FPerformanceThreshold Thresholds;
    Thresholds.MinFPS = 60.0f;
    Thresholds.MaxFrameTimeMs = 16.67f;
    Thresholds.HitchThresholdMs = 33.0f;
    UArgusLens::SetPerformanceThresholds(Thresholds);
    
    // Run benchmark
    UArgusLens::StartPerformanceMonitoring(30.0f);
    SimulateGameplay(30.0f);
    UArgusLens::StopPerformanceMonitoring();
    
    // Validate results
    if (!UArgusLens::DidPassPerformanceGates()) {
        float AvgFPS = UArgusLens::GetAverageFPS();
        int32 Hitches = UArgusLens::GetHitchCount();
        UE_LOG(LogTemp, Error, TEXT("FAILED: %.1f FPS, %d hitches"), AvgFPS, Hitches);
        return false;
    }
    
    // Export report
    UArgusLens::ExportPerformanceArtifact();
    return true;
}
```

---

### Example 2: Memory Leak Detection

```cpp
// MemoryLeakTest.cpp
NEXUS_TEST(FMemoryLeakTest, "Performance.Memory.NoLeaks", ETestPriority::High)
{
    // Set strict memory budget
    FPerformanceThreshold Thresholds;
    Thresholds.MaxMemoryMb = 512.0f;  // Tight budget to catch leaks
    UArgusLens::SetPerformanceThresholds(Thresholds);
    
    // Allocate and deallocate repeatedly
    UArgusLens::StartPerformanceMonitoring(60.0f);
    
    for (int32 i = 0; i < 1000; ++i) {
        // This should not leak
        AActor* TempActor = GetWorld()->SpawnActor<AActor>();
        TempActor->Destroy();
    }
    
    UArgusLens::StopPerformanceMonitoring();
    
    // Check for memory growth
    float PeakMemory = UArgusLens::GetPeakMemoryMb();
    if (PeakMemory > 512.0f) {
        UE_LOG(LogTemp, Error, TEXT("❌ Possible memory leak: Peak %.0f MB"), PeakMemory);
        return false;
    }
    
    UE_LOG(LogTemp, Display, TEXT("✅ Memory test passed: Peak %.0f MB"), PeakMemory);
    return true;
}
```

---

### Example 3: Frame Rate Under Load

```cpp
// FrameRateStressTest.cpp
NEXUS_TEST(FFrameRateStressTest, "Performance.Stress.MaxPlayers", ETestPriority::High)
{
    // Spawn many players/NPCs
    const int32 PlayerCount = 32;
    const int32 NPCCount = 128;
    
    // Monitor performance
    FPerformanceThreshold Thresholds;
    Thresholds.MinFPS = 30.0f;  // Lower bar under stress
    UArgusLens::SetPerformanceThresholds(Thresholds);
    
    UArgusLens::StartPerformanceMonitoring(120.0f);  // 2 minute test
    
    // Spawn all entities
    SpawnPlayers(PlayerCount);
    SpawnNPCs(NPCCount);
    
    // Keep running, collecting metrics
    FPlatformProcess::Sleep(120.0f);
    
    UArgusLens::StopPerformanceMonitoring();
    
    // Generate detailed report
    if (UArgusLens::DidPassPerformanceGates()) {
        UE_LOG(LogTemp, Display, TEXT("✅ Stress test PASSED at %d players + %d NPCs"),
            PlayerCount, NPCCount);
    } else {
        float AvgFPS = UArgusLens::GetAverageFPS();
        UE_LOG(LogTemp, Error, TEXT("❌ Stress test FAILED: %.1f FPS average"), AvgFPS);
    }
    
    UArgusLens::ExportPerformanceArtifact();
    return UArgusLens::DidPassPerformanceGates();
}
```

---

## Report Formats

### JSON Report

```json
{
  "test": "Performance.Desktop.60FPS",
  "timestamp": "2026-01-01T14:30:45Z",
  "duration_seconds": 30.0,
  "samples": 1800,
  "fps": {
    "average": 62.3,
    "min": 45.2,
    "max": 65.0,
    "p95": 63.5,
    "passed": true
  },
  "memory": {
    "initial_mb": 512.0,
    "peak_mb": 1024.5,
    "final_mb": 1023.2,
    "passed": true
  },
  "hitches": {
    "count": 2,
    "threshold_ms": 33.0,
    "max_hitch_ms": 48.5,
    "passed": true
  },
  "overall": {
    "passed": true,
    "status": "PASS"
  }
}
```

### CSV Report

```csv
frame_number,timestamp,fps,frame_time_ms,memory_mb,is_hitch
1,0.000,60.0,16.67,512.0,false
2,0.017,60.5,16.53,512.5,false
3,0.033,60.2,16.60,513.0,false
...
48,0.800,45.2,22.12,520.0,true
...
1800,30.000,62.3,16.04,1024.5,false
```

### HTML Report

Self-contained dashboard with:
- FPS graph with min/avg/max
- Memory usage timeline
- Hitch summary and distribution
- Performance gates status
- Exportable data tables

---

## Integration with Nexus Testing Framework

**New in Nexus 2.0:** ArgusLens metrics are automatically captured during game-thread tests and provided in the test context for assertions.

### Using NEXUS_PERF_TEST Macro

```cpp
#include "Nexus/Core/Public/NexusCore.h"

// Performance test with automatic ArgusLens integration
NEXUS_PERF_TEST(FNetworkLatencyTest, "Network.Performance.Latency", ETestPriority::High)
{
    const FNexusTestContext& Context = /* provided by macro */;
    
    // Simulate network traffic
    SimulateNetworkLoad(100);  // 100 requests/sec
    FPlatformProcess::Sleep(5.0f);  // Run for 5 seconds
    
    // Assert performance metrics from ArgusLens
    return Context.AssertAverageFPS(30.0f) &&
           Context.AssertMaxMemory(1024.0f) &&
           Context.AssertMaxHitches(100, 33.0f);
}
```

### Performance Assertion Helpers

Available on `FNexusTestContext` when using `NEXUS_PERF_TEST`:

```cpp
// Assert minimum average FPS
bool bFPSGood = Context.AssertAverageFPS(60.0f);

// Assert maximum memory usage (MB)
bool bMemoryGood = Context.AssertMaxMemory(2048.0f);

// Assert maximum hitch count with threshold (ms)
bool bHitchesGood = Context.AssertMaxHitches(5, 33.0f);

// Check if performance data was collected
bool bHasData = Context.HAS_PERF_DATA();
```

### Complete Example: Performance Gate Test

```cpp
// PerformanceGateWithNexus.cpp
NEXUS_PERF_TEST(FComplexSceneTest, "Performance.Scene.MaxActors", ETestPriority::High)
{
    const FNexusTestContext& Context = /* auto-provided */;
    
    // Verify we're in a valid game world
    if (!Context.IsValid()) {
        UE_LOG(LogTemp, Warning, TEXT("Skipping: No game world context"));
        return true;  // Gracefully skip
    }
    
    // Spawn a complex scene
    for (int32 i = 0; i < 500; ++i) {
        AActor* Actor = Context.SpawnTestCharacter();
        if (!Actor) break;  // Reached spawn limit
    }
    
    // Wait for physics and rendering to stabilize
    FPlatformProcess::Sleep(2.0f);
    
    // Assert performance under load
    bool bPassed = 
        Context.AssertAverageFPS(30.0f) &&        // Minimum 30 FPS
        Context.AssertMaxMemory(2048.0f) &&       // Under 2GB
        Context.AssertMaxHitches(10, 50.0f);      // Max 10 hitches
    
    // Automatic cleanup of spawned actors happens when context goes out of scope
    return bPassed;
}
```

### Migration from Manual ArgusLens to NEXUS_PERF_TEST

**Before:**
```cpp
NEXUS_TEST(FTest, "Performance.Test", ETestPriority::Normal)
{
    FPerformanceThreshold Thresholds;
    Thresholds.MinFPS = 60.0f;
    UArgusLens::SetPerformanceThresholds(Thresholds);
    UArgusLens::StartPerformanceMonitoring(30.0f);
    // ... test code ...
    UArgusLens::StopPerformanceMonitoring();
    return UArgusLens::DidPassPerformanceGates();
}
```

**After:**
```cpp
NEXUS_PERF_TEST(FTest, "Performance.Test", ETestPriority::Normal)
{
    const FNexusTestContext& Context = /* auto-provided */;
    // ... test code ...
    return Context.AssertAverageFPS(60.0f);  // Simpler!
}
```

---

## Performance Considerations

### Sampling Overhead
- **Per-frame cost:** ~0.1-0.2ms
- **Memory per sample:** ~40 bytes
- **Typical overhead for 60s test at 60 FPS:** ~2.8 MB

### Best Practices

**DO:**
- Set realistic thresholds for your platform
- Monitor during actual gameplay scenarios
- Export reports for historical comparison
- Use `GetCurrentPerformanceSnapshot()` for mid-test assertions

**DON'T:**
- Monitor extremely long periods (>10 minutes) without pagination
- Set thresholds tighter than physics timestep (16.67ms at 60 FPS)
- Call performance monitoring functions from other threads

---

## Troubleshooting

### Results Show 0 FPS

**Problem:** `GetAverageFPS()` returns 0 or very low values

**Solutions:**
1. Verify `StopPerformanceMonitoring()` was called
2. Check that game was actually running during monitoring period
3. Ensure game loop was actively updating frames

### Memory Value Seems Wrong

**Problem:** Reported memory doesn't match Task Manager

**Solutions:**
1. ArgusLens reports GPU memory + CPU heap
2. Task Manager shows process memory (includes overhead)
3. Values may differ by 10-20% due to timing
4. Trends are more important than absolute values

### No Hitches Detected (When Expected)

**Problem:** `GetHitchCount()` returns 0 despite visible stuttering

**Solutions:**
1. Increase `HitchThresholdMs` (default 100ms is generous)
2. Verify `StartPerformanceMonitoring()` was actually called
3. Check that gameplay code isn't artificially capping FPS

---

## See Also

- [OBSERVER_NETWORK_GUIDE.md](OBSERVER_NETWORK_GUIDE.md) — Logging performance events
- [Nexus Core Framework](./NEXUS_GUIDE.md)
- [FringeNetwork Chaos Testing](./FRINGENETWORK_GUIDE.md)
