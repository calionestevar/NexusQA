# Palantír: Test Observability & Distributed Tracing

This document explains the **Palantír subsystem** within Nexus — a production-grade correlation ID and rich assertion system inspired by Cerner's test identifier framework and Garmin's DataDog tracing patterns.

**What is Palantír?** In Tolkien lore, a *palantír* is a seeing-stone that lets you observe events across vast distances. Similarly, our Palantír subsystem provides three capabilities:

### 🔮 PalantírTrace — Distributed Tracing
Sees across systems and time. Provides unique correlation IDs that flow through logs, HTTP headers, and game industry observability tools (Sentry, PlayFab, GameAnalytics, Unreal Insights).

**Key Features:**
- UUID generation (`nexus-test-<UUID>`)
- Thread-local trace context
- Breadcrumb timeline tracking
- JSON export for external systems
- Automatic injection via `UE_LOG_TRACE` macro

### 👁️ PalantírVision — Rich Assertions  
Sees hidden context at the moment of failure. Captures game state, performance metrics, and environment details when assertions fail.

**Key Features:**
- Fluent builder API (`NEXUS_ASSERT_GT().WithContext().ExecuteOrFail()`)
- Automatic context capture (trace ID, timestamp, callstack)
- Integration with ArgusLens performance metrics
- JSON export for assertion forensics

### 🔵 PalantírOracle — Result Aggregation
Sees outcomes and patterns. Aggregates test results in-process for live dashboards and LCARS reporting.

**Key Features:**
- Real-time result tracking
- Pass/fail/skip statistics
- Integration with LCARS HTML reporter

## Overview

**Problem:** When tests fail, you see:
```
[14:32:15] Test failed: Assertion X
```

**Where are the logs?** How do you correlate with:
- Backend API errors?
- DataDog events?
- Network chaos injection events?
- Performance anomalies?

**Solution:** Every test gets a **unique trace ID** that's injected into:
- Game logs (via `UE_LOG_TRACE`)
- HTTP headers (User-Agent, X-Trace-ID)
- Performance metrics (ArgusLens)
- Network events (Chaos module)
- JSON artifacts

Now you can grep a single trace ID across **all systems** and reconstruct the full sequence of events.

---

## Core Concepts

### Trace ID
A **unique correlation ID** for each test execution.

**Format:** `nexus-test-<UUID>`  
**Example:** `nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e`

**Generated automatically** at test start; available via `FPalantirTrace::GetCurrentTraceID()`.

### Breadcrumbs
**Timeline events** recorded during test execution.

**Example:**
```
[0.000s] Init: Test started
[0.050s] LoadAsset: Loading player character
[0.150s] SpawnEntity: Created test player
[0.200s] ExecuteLogic: Running assertions
[0.450s] Cleanup: Tearing down test world
```

**Use case:** When a test fails, the breadcrumb timeline shows *exactly* when and where things went wrong.

### Trace Context
**Thread-local storage** for trace ID + breadcrumbs. Automatically cleared after test completion.

**RAII pattern:** `FPalantirTraceGuard` automatically initializes and cleans up.

---

## Usage

### Basic Test with Tracing

```cpp
#include "NexusTest.h"
#include "PalantirTrace.h"

NEXUS_TEST(FMyTest, "MyModule.Feature.Test", ETestPriority::Normal)
{
    FPalantirTraceGuard Guard;  // Auto-generates trace ID
    
    FString TraceID = Guard.GetTraceID();
    UE_LOG(LogMyModule, Log, TEXT("Running test with trace: %s"), *TraceID);
    
    // Your test logic
    return true;
}
```

**Output:**
```
[LogPalantirTrace] Trace started: nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e
[LogMyModule] Running test with trace: nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e
[LogPalantirTrace] Trace ended: nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e (duration: 0.45s)
```

### Injecting Trace ID into Logs

Use `UE_LOG_TRACE` instead of `UE_LOG`:

```cpp
// Before (no correlation)
UE_LOG(LogMyModule, Log, TEXT("Asset loaded"));
// Output: [LogMyModule] Asset loaded

// After (with correlation)
UE_LOG_TRACE(LogMyModule, Log, TEXT("Asset loaded"));
// Output: [LogMyModule] [nexus-test-a3f2...] Asset loaded
```

### Recording Breadcrumbs

Breadcrumbs are timeline markers for event reconstruction:

```cpp
FPalantirTraceGuard Guard;

PALANTIR_BREADCRUMB(TEXT("Init"), TEXT("Starting test"));
// [LogPalantirTrace] [nexus-test-...] [0.000s] Init: Starting test

MyAssetLoader->Load(TEXT("TestAsset"));
PALANTIR_BREADCRUMB(TEXT("LoadAsset"), FString::Printf(TEXT("Loaded: TestAsset")));
// [LogPalantirTrace] [nexus-test-...] [0.150s] LoadAsset: Loaded: TestAsset

ExecuteGameLogic();
PALANTIR_BREADCRUMB(TEXT("LogicComplete"), TEXT(""));
// [LogPalantirTrace] [nexus-test-...] [0.300s] LogicComplete:
```

### Exporting Trace to JSON

```cpp
FPalantirTraceGuard Guard;

// ... test code ...

FString TraceJSON = FPalantirTrace::ExportToJSON();
// {
//   "trace_id": "nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e",
//   "start_time": 1701961935.123,
//   "duration_seconds": 0.45,
//   "breadcrumbs": [
//     {"timestamp": 0.0, "event": "[0.000s] Init: Starting test"},
//     {"timestamp": 0.15, "event": "[0.150s] LoadAsset: Loaded: TestAsset"},
//     {"timestamp": 0.3, "event": "[0.300s] LogicComplete:"}
//   ]
// }
```

---

## Enhanced Assertions

The Palant�r subsystem provides rich assertion macros with automatic context capture.

### Basic Assertions

```cpp
float CurrentFPS = GetFPS();

// Before: Generic failure message
CHECK(CurrentFPS > 30.0f);  // ← "Check failed"

// After: Rich context
FAssertionContext(TEXT("FPS > 30"), __FILE__, __LINE__)
    .WithContext(TEXT("Current FPS"), CurrentFPS)
    .WithContext(TEXT("Threshold"), 30.0f)
    .WithHint(TEXT("Check ArgusLens for performance spikes"))
    .ExecuteOrFail();

// Output:
// [LogPalantirVision] Assertion Failed: FPS > 30
// [LogPalantirVision] Location: MyTest.cpp(42)
// [LogPalantirVision] Trace ID: nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e
// [LogPalantirVision] Hint: Check ArgusLens for performance spikes
// [LogPalantirVision] Context:
// [LogPalantirVision]   Current FPS: 25.00
// [LogPalantirVision]   Threshold: 30.00
```

### Comparison Macros

```cpp
// Greater-than assertion
NEXUS_ASSERT_GT(CurrentFPS, 30.0f)
    .WithContext(TEXT("Scene"), *CurrentLevelName())
    .WithHint(TEXT("Check for GC spikes"))
    .ExecuteOrFail();

// Less-than assertion
NEXUS_ASSERT_LT(FrameTimeMs, 16.67f)  // 60 FPS target
    .WithPerformanceData()
    .ExecuteOrFail();

// Equality assertion
NEXUS_ASSERT_EQ(PlayerHealth, ExpectedHealth)
    .WithContext(TEXT("DamageTaken"), DamageTaken)
    .ExecuteOrFail();

// Boolean assertions
NEXUS_ASSERT_TRUE(World->IsValid())
    .WithHint(TEXT("World pointer is null"))
    .ExecuteOrFail();

NEXUS_ASSERT_FALSE(bIsPlayerDead)
    .WithContext(TEXT("Health"), CurrentHealth)
    .ExecuteOrFail();
```

### Fluent Builder Pattern

```cpp
AssertThat(PlayerInventoryCount)
    .IsGreaterThan(0)
    .WithContext(TEXT("Level"), CurrentLevel)
    .WithContext(TEXT("Difficulty"), GameDifficulty)
    .WithHint(TEXT("Check item pickup logic"))
    .ExecuteOrFail();
```

---

## Integration with External Systems

### DataDog (APM)

**Inject trace ID into HTTP headers** (for backend correlation):

```cpp
// In your network request handler:
FString TraceID = FPalantirTrace::GetCurrentTraceID();

// Add to HTTP headers
RequestHeaders.AddCustomHeader(TEXT("X-Trace-ID"), TraceID);
RequestHeaders.AddCustomHeader(TEXT("User-Agent"), 
    FString::Printf(TEXT("NexusTest/%s"), *TraceID));
```

**In your backend:**
```
GET /api/players/123 HTTP/1.1
X-Trace-ID: nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e
User-Agent: NexusTest/nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e
```

**In DataDog**, use the trace ID to correlate:
- Game logs
- Network requests
- Backend logs
- Database queries
- Performance events

### ELK Stack (Elasticsearch/Logstash/Kibana)

**Export traces as JSON** for Logstash ingestion:

```cpp
// In Nexus reporter:
FString TraceJSON = FPalantirTrace::ExportToJSON();
SaveTraceToFile(TEXT("Saved/NexusReports/traces.jsonl"), TraceJSON);
```

**Logstash config:**
```
input {
  file {
    path => "/game/Saved/NexusReports/traces.jsonl"
    codec => json
  }
}

filter {
  mutate { add_field => {"[@metadata][index_name]" => "nexus-traces"} }
}

output {
  elasticsearch {
    hosts => ["localhost:9200"]
    index => "nexus-traces-%{+YYYY.MM.dd}"
  }
}
```

**In Kibana:** Filter/search by `trace_id` to reconstruct test timeline.

### Splunk

**Forward trace logs to Splunk:**

```cpp
// Structured logging (for Splunk Heavy Forwarder):
UE_LOG_TRACE(LogNexus, Log, 
    TEXT("event=test_complete trace_id=%s duration=%.2f status=PASSED"),
    *TraceID, Duration);
```

**In Splunk:**
```
source=game_logs event=test_complete | stats count by trace_id, status
```

### Custom Log Aggregation

**Grep logs by trace ID:**

```bash
# Find all logs for a specific test
grep "nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e" /var/log/game/*.log

# Count events per trace
grep "nexus-test-" /var/log/game/*.log | cut -d' ' -f2 | sort | uniq -c

# Export traces for analysis
grep -h "nexus-test-" /var/log/game/*.log > traces.txt
cat traces.txt | jq -s 'group_by(.trace_id)'
```

---

## Best Practices

### ✅ Do

- **Use `FPalantirTraceGuard`** at the start of every test (automatic cleanup)
- **Add breadcrumbs** at major test milestones (asset loading, logic execution, cleanup)
- **Enhance assertions** with context (current state, expected behavior, hints)
- **Log via `UE_LOG_TRACE`** for automatic trace ID injection
- **Export traces to JSON** for external system ingestion
- **Use structured logging** (JSON format) for parseable logs

### ❌ Don't

- **Manually call `FPalantirTrace::Clear()`** — use `FPalantirTraceGuard` instead
- **Ignore assertion failures** — use `ExecuteOrFail()` to propagate errors
- **Log sensitive data in breadcrumbs** — (PII, API keys, passwords)
- **Add too many breadcrumbs** — keep them to major events only
- **Forget to import `TraceContext.h`** in your test files

---

## Sample Output

### Console Log
```
[2025-12-07 14:32:15.123] [LogPalantirTrace] Trace started: nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e
[2025-12-07 14:32:15.234] [LogMyTest] [nexus-test-a3f2...] Starting test execution
[2025-12-07 14:32:15.345] [LogPalantirTrace] [nexus-test-a3f2...] [0.222s] Init: Loading test level
[2025-12-07 14:32:15.678] [LogPalantirTrace] [nexus-test-a3f2...] [0.555s] AssetLoad: Loaded 12 assets
[2025-12-07 14:32:16.012] [LogPalantirTrace] [nexus-test-a3f2...] [0.889s] ExecuteLogic: Running test assertions
[2025-12-07 14:32:16.234] [LogMyTest] [nexus-test-a3f2...] All assertions passed
[2025-12-07 14:32:16.345] [LogPalantirTrace] Trace ended: nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e (duration: 1.22s)
```

### Exported JSON Trace
```json
{
  "trace_id": "nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e",
  "start_time": 1701961935.123,
  "duration_seconds": 1.222,
  "breadcrumbs": [
    {
      "timestamp": 0.222,
      "event": "[0.222s] Init: Loading test level"
    },
    {
      "timestamp": 0.555,
      "event": "[0.555s] AssetLoad: Loaded 12 assets"
    },
    {
      "timestamp": 0.889,
      "event": "[0.889s] ExecuteLogic: Running test assertions"
    }
  ]
}
```

### Assertion Failure with Context
```json
{
  "assertion": "FPS > 30",
  "file": "MyTest.cpp",
  "line": 42,
  "trace_id": "nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e",
  "passed": false,
  "hint": "Check ArgusLens for performance spikes",
  "context": {
    "Current FPS": "25.00",
    "Threshold": "30.00",
    "GPU Load": "85.5%",
    "Memory": "2048 MB"
  }
}
```

---

## Architecture

```
┌─────────────────────────────────────┐
│  Test Execution (Nexus)             │
└──────────────┬──────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│  FPalantirTraceGuard                  │
│  - Auto-generates trace ID           │
│  - Initializes thread-local storage  │
│  - Sets up breadcrumb tracking       │
└──────────────┬───────────────────────┘
               │
        ┌──────┴──────┬────────────┬───────────┐
        │             │            │           │
        ▼             ▼            ▼           ▼
   ┌─────────┐  ┌──────────┐  ┌─────────┐  ┌────────────┐
   │UE_LOG   │  │Assertions│  │Breadcrm │  │HTTP Headers│
   │_TRACE   │  │          │  │bs       │  │(DataDog)   │
   └─────────┘  └──────────┘  └─────────┘  └────────────┘
        │             │            │           │
        └──────────────┴────────────┴───────────┘
               │
               ▼
    ┌──────────────────────────────┐
    │  FPalantirTrace::ExportToJSON │
    └──────────────┬───────────────┘
               │
   ┌───────────┼───────────┐
   ▼           ▼           ▼
┌─────┐  ┌───────┐  ┌──────────┐
│LCARS│  │DataDog│  │ELK/Splunk│
│JSON │  │(APM)  │  │(Aggregate│
└─────┘  └───────┘  └──────────┘
```

---

## Further Reading

- [ARCHITECTURE.md](../ARCHITECTURE.md) — Framework design
- [BUILD.md](../BUILD.md) — Build & test instructions
- [CONTRIBUTING.md](../CONTRIBUTING.md) — Development standards
- **Test examples:** `Source/Observability/Private/Tests/Observability.test.cpp`
