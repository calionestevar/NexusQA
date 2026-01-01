# ObserverNetworkDashboard Usage Guide

## Overview

The **ObserverNetworkDashboard** is a real-time monitoring and auditing system for the NexusQA testing framework. It provides live visibility into test execution, safety events, and system stability through an ImGui-based dashboard and generates web-based reports.

Named after the Observer Network concept from "Fringe," it watches for anomalies, safety violations, and testing events across your game's reality (aka your test suite).

---

## Features

### 📊 Live Dashboard (ImGui)
- **Real-time event logging** - See safety events as they occur
- **Session uptime tracking** - Monitor how long tests have been running
- **Event counters** - Track blocked/passed safety checks
- **Color-coded status** - Green for blocked (safe), red for failures
- **Thread-safe rendering** - No locks held during ImGui calls

### 📋 Web Report Generation
- **HTML report export** - Beautiful formatted report with charts
- **Event timeline** - Chronological log of all safety events
- **Summary statistics** - Pass/fail counts and session metadata
- **Shareable output** - Reports saved to `Saved/Reports/` directory

### 🔒 Thread-Safe Architecture
- **Critical section protection** - All state access protected by mutex
- **Lock-free rendering** - Dashboard data copied under lock, rendered lock-free
- **Concurrent safety** - Multiple systems can log events simultaneously

---

## API Reference

### Initialization

```cpp
void UObserverNetworkDashboard::Initialize();
```

**Call once at game startup or before tests begin.**

```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the dashboard
    UObserverNetworkDashboard::Initialize();
}
```

---

### Logging Safety Events

```cpp
void UObserverNetworkDashboard::LogSafetyEvent(
    const FString& EventType, 
    const FString& Details
);
```

**Call whenever a safety-relevant event occurs.**

**Examples:**

```cpp
// In a compliance check
if (bViolatesCOPPA)
{
    UObserverNetworkDashboard::LogSafetyEvent(
        TEXT("COPPA_VIOLATION"), 
        TEXT("User age < 13 without parental consent")
    );
}

// In a network test
if (PacketLossDectected)
{
    UObserverNetworkDashboard::LogSafetyEvent(
        TEXT("NETWORK_PACKET_LOSS"),
        FString::Printf(TEXT("%.1f%% loss detected"), PacketLossPercent)
    );
}

// In chaos injection
UObserverNetworkDashboard::LogSafetyEvent(
    TEXT("CHAOS_INJECTION_BLOCKED"),
    TEXT("Attempted DuplicateCameraBoom injection was blocked by safety rules")
);

// When a test passes
UObserverNetworkDashboard::LogSafetyEvent(
    TEXT("TEST_PASSED"),
    TEXT("Performance test: FPS >= 60 for 30 seconds")
);
```

**Event Type Naming Convention:**

Use `[CATEGORY]_[RESULT]` format:
- `COPPA_VIOLATION` / `COPPA_PASSED`
- `GDPR_VIOLATION` / `GDPR_PASSED`
- `NETWORK_LATENCY_HIGH` / `NETWORK_STABLE`
- `CHAOS_INJECTION_BLOCKED` / `CHAOS_INJECTION_ACTIVE`
- `TEST_PASSED` / `TEST_FAILED`

---

### Live Dashboard Updates

```cpp
void UObserverNetworkDashboard::UpdateLiveDashboard();
```

**Call every frame while dashboard is active (e.g., in Tick).**

**Example:**

```cpp
void ATestGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update dashboard every frame
    UObserverNetworkDashboard::UpdateLiveDashboard();
}
```

**Requirements:**
- ImGui must be enabled: `WITH_IMGUI=1` in your build configuration
- Engine must have a valid `GameViewport`
- Only renders when the Unreal Engine window is focused

---

### Generate Web Report

```cpp
void UObserverNetworkDashboard::GenerateWebReport();
```

**Call when tests complete to save a web report.**

**Example:**

```cpp
void ATestGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // Generate final report
    UObserverNetworkDashboard::GenerateWebReport();
}
```

**Output:**
- Location: `[Project]/Saved/Reports/observer_network_TIMESTAMP.html`
- Format: Self-contained HTML file with embedded CSS
- Content: Complete event timeline with statistics

---

## Integration Examples

### Example 1: Basic Game Mode Integration

```cpp
// MyGameMode.h
UCLASS()
class MYGAME_API AMyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
```

```cpp
// MyGameMode.cpp
#include "MyGameMode.h"
#include "FringeNetwork/Public/ObserverNetworkDashboard.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    UObserverNetworkDashboard::Initialize();
    UE_LOG(LogTemp, Warning, TEXT("🔍 Observer Network initialized"));
}

void AMyGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UObserverNetworkDashboard::UpdateLiveDashboard();
}

void AMyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    UObserverNetworkDashboard::GenerateWebReport();
    UE_LOG(LogTemp, Warning, TEXT("✅ Observer Network report generated"));
}
```

---

### Example 2: Compliance Testing

```cpp
// ComplianceChecker.cpp
#include "ComplianceChecker.h"
#include "FringeNetwork/Public/ObserverNetworkDashboard.h"

void UComplianceChecker::CheckCOPPA(const FUserProfile& UserProfile)
{
    if (UserProfile.bIsUnderThirteen && !UserProfile.bHasParentalConsent)
    {
        UObserverNetworkDashboard::LogSafetyEvent(
            TEXT("COPPA_VIOLATION"),
            FString::Printf(TEXT("User %d is under 13 without parental consent"), 
                UserProfile.UserId)
        );
        return false;
    }

    UObserverNetworkDashboard::LogSafetyEvent(
        TEXT("COPPA_PASSED"),
        TEXT("User age/consent validation successful")
    );
    return true;
}

void UComplianceChecker::CheckGDPR(const FUserData& UserData)
{
    if (UserData.bPIIIncluded && !UserData.bPIIConsented)
    {
        UObserverNetworkDashboard::LogSafetyEvent(
            TEXT("GDPR_VIOLATION"),
            TEXT("PII present without explicit consent")
        );
        return false;
    }

    UObserverNetworkDashboard::LogSafetyEvent(
        TEXT("GDPR_PASSED"),
        TEXT("PII handling compliant with GDPR")
    );
    return true;
}
```

---

### Example 3: Performance Monitoring

```cpp
// PerformanceMonitor.cpp
#include "PerformanceMonitor.h"
#include "FringeNetwork/Public/ObserverNetworkDashboard.h"
#include "ArgusLens/Public/ArgusLens.h"

void UPerformanceMonitor::CheckFrameRate()
{
    float CurrentFPS = UArgusLens::GetAverageFPS();
    const float MinFPS = 60.0f;

    if (CurrentFPS < MinFPS)
    {
        UObserverNetworkDashboard::LogSafetyEvent(
            TEXT("PERFORMANCE_LOW"),
            FString::Printf(TEXT("FPS %.1f below threshold %.1f"), 
                CurrentFPS, MinFPS)
        );
    }
    else
    {
        UObserverNetworkDashboard::LogSafetyEvent(
            TEXT("PERFORMANCE_STABLE"),
            FString::Printf(TEXT("FPS %.1f meets target %.1f"), 
                CurrentFPS, MinFPS)
        );
    }
}

void UPerformanceMonitor::CheckMemory()
{
    float PeakMemory = UArgusLens::GetPeakMemoryMb();
    const float MaxMemory = 2048.0f;  // 2GB

    if (PeakMemory > MaxMemory)
    {
        UObserverNetworkDashboard::LogSafetyEvent(
            TEXT("MEMORY_EXCEEDED"),
            FString::Printf(TEXT("Memory %.0f MB exceeds limit %.0f MB"), 
                PeakMemory, MaxMemory)
        );
    }
}
```

---

## Dashboard Rendering

### Enabling ImGui Rendering

The dashboard requires ImGui plugin:

**In your `.Build.cs` file:**
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "ImGui",  // Required for dashboard rendering
});
```

**In your project settings:**
1. Edit > Project Settings > Plugins
2. Search for "ImGui"
3. Enable "ImGui Unreal Engine Integration"
4. Restart editor

### Dashboard Appearance

When running with ImGui enabled, the dashboard displays:

```
┌─ OBSERVER NETWORK — LIVE AUDIT ─────────────────────┐
│ REALITY STATUS: STABLE                              │
│─────────────────────────────────────────────────────│
│ Session Time: 42.5 seconds                          │
│─────────────────────────────────────────────────────│
│ SAFETY EVENTS                                       │
│ ✓ COPPA_PASSED                    : 3              │
│ ✓ GDPR_PASSED                     : 2              │
│ ✓ PERFORMANCE_STABLE              : 15             │
│ ✗ NETWORK_LATENCY_HIGH            : 1              │
│─────────────────────────────────────────────────────│
│ RECENT EVENTS                                       │
│  [   0.23] COPPA_PASSED: User age verified        │
│  [   0.45] GDPR_PASSED: Consent records intact     │
│  [  10.12] PERFORMANCE_STABLE: FPS 62 meets goal   │
│  [  42.10] NETWORK_LATENCY_HIGH: 250ms detected    │
└─────────────────────────────────────────────────────┘
```

---

## Web Report Format

Generated reports follow this structure:

```html
<!DOCTYPE html>
<html>
<head>
    <title>Observer Network Report</title>
    <style>
        /* LCARS-themed styling with orange/blue theme */
    </style>
</head>
<body>
    <div class="header">
        <h1>🔍 OBSERVER NETWORK AUDIT REPORT</h1>
        <p>Timestamp: 2026-01-01 14:30:45</p>
        <p>Session Duration: 42.5 seconds</p>
    </div>

    <div class="summary">
        <h2>Executive Summary</h2>
        <table>
            <tr><td>Total Events</td><td>21</td></tr>
            <tr><td>Passed Checks</td><td>18</td></tr>
            <tr><td>Failed Checks</td><td>3</td></tr>
            <tr><td>Overall Status</td><td>WARN</td></tr>
        </table>
    </div>

    <div class="timeline">
        <h2>Event Timeline</h2>
        <table>
            <tr><th>Time (s)</th><th>Event</th><th>Details</th></tr>
            <tr><td>0.23</td><td>COPPA_PASSED</td><td>User age verified</td></tr>
            <!-- More events... -->
        </table>
    </div>
</body>
</html>
```

---

## Best Practices

### ✅ DO

- **Initialize at game startup** - Call `Initialize()` in `BeginPlay()`
- **Update every frame** - Call `UpdateLiveDashboard()` in `Tick()` 
- **Log meaningful events** - Use descriptive event types and details
- **Generate report at shutdown** - Call `GenerateWebReport()` in `EndPlay()`
- **Use consistent naming** - Follow `[CATEGORY]_[RESULT]` convention
- **Include context in details** - Add numeric values, usernames, etc.

### ❌ DON'T

- **Don't hold locks during rendering** - The implementation copies data under lock
- **Don't spam events** - Log only meaningful state changes, not every frame
- **Don't call Generate from hot paths** - Only call at strategic shutdown points
- **Don't assume ImGui is available** - Always check `WITH_IMGUI` preprocessor

---

## Troubleshooting

### Dashboard Not Showing

**Problem:** ImGui window doesn't appear during gameplay

**Solutions:**
1. Verify ImGui plugin is enabled: `Plugins > Search "ImGui" > Enable`
2. Check `WITH_IMGUI` is set to 1 in your build
3. Ensure `GameViewport` exists (dashboard checks for it)
4. Try pressing `Shift+F1` to toggle UI visibility

### Events Not Logging

**Problem:** `LogSafetyEvent()` calls don't appear

**Solutions:**
1. Verify `Initialize()` was called first
2. Check log output level includes `Warning` category
3. Ensure events are logged from game thread (not async tasks)
4. Check that details string is not empty

### Report Not Generated

**Problem:** `GenerateWebReport()` doesn't create output file

**Solutions:**
1. Verify `Saved/` directory exists in project root
2. Check write permissions on output directory
3. Ensure at least one event was logged before generating
4. Check `Saved/Reports/` directory for output file

---

## Performance Considerations

### Memory Usage
- **Event log array** - Stores one FString per event (~128 bytes each)
- **Event counters** - One entry per unique event type (~40 bytes)
- **Typical overhead** - ~50KB for 500 events

### Thread Safety
- **Mutex type** - `FCriticalSection` (lightweight)
- **Lock scope** - Minimal (only for data access, not rendering)
- **No deadlocks** - Dashboard copies data, renders lock-free

### ImGui Overhead
- **Rendering** - Only when `UpdateLiveDashboard()` is called
- **CPU cost** - ~0.5-1ms per frame with 100+ events
- **GPU cost** - Minimal (rendered to engine viewport)

---

## See Also

- [NexusQA Overview](../README.md)
- [Compliance Testing Guide](./COMPLIANCE_TESTING.md)
- [Performance Monitoring](./ARGUSLENS_GUIDE.md)
- [Network Testing](./FRINGENETWORK_GUIDE.md)
