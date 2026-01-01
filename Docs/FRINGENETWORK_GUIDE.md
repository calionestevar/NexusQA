# FringeNetwork — Network Chaos Engineering & Observer Patterns

**FringeNetwork** provides tools for testing network resilience, simulating adverse conditions, and coordinating distributed test observers. Named after the Fringe division's ability to observe parallel realities, this module helps you test how your game handles network failures, latency, and multi-region deployments.

---

## Overview

### Core Capabilities

| Feature | Purpose | Use Case |
|---------|---------|----------|
| **Lag Injection** | Simulate network latency | Test timeout handling, see player experience at 200ms+ ping |
| **Packet Loss Simulation** | Drop packets randomly | Validate resend logic, ensure graceful degradation |
| **Disconnect Testing** | Force connection drops | Test reconnection flows, session recovery |
| **Observer Network** | Real-time dashboard | Monitor test events, audit safety checks |
| **Parallel Realm Testing** | Multi-region validation | Test geo-redundancy, regional server failover |

---

## Quick Start

### 1. Add FringeNetwork to Your Project

**In your `.Build.cs` file:**

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "FringeNetwork",
    "HTTP",
    "Json",
    "JsonUtilities"
});
```

### 2. Activate Network Testing

**In your game code:**

```cpp
#include "FringeNetwork/Public/FringeNetwork.h"

// In your game mode or initialization code
void MyInitialization()
{
    // Activate the observer network with live dashboard
    FFringeNetwork::ActivateObserverNetwork();
    
    // Run tests on primary server and replicas
    FFringeNetwork::RunObserverNetworkTests();
    
    // Test multiple geographic regions
    FFringeNetwork::TestParallelRealms();
}
```

---

## API Reference

### Observer Network Activation

```cpp
void FFringeNetwork::ActivateObserverNetwork();
```

Initializes the real-time dashboard and starts monitoring network events.

**Example:**

```cpp
void ANetworkTestGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize observer dashboard
    FFringeNetwork::ActivateObserverNetwork();
    
    UE_LOG(LogTemp, Warning, TEXT("🌐 FringeNetwork: Observer network activated"));
}
```

**What it does:**
- Initializes `UObserverNetworkDashboard`
- Sets up HTTP module for network requests
- Prepares for event logging and monitoring

---

### Running Observer Network Tests

```cpp
void FFringeNetwork::RunObserverNetworkTests();
```

Executes tests against your primary server and replica servers with automatic HTTP requests.

**Example:**

```cpp
void ANetworkTestGameMode::BeginTestPhase()
{
    UE_LOG(LogTemp, Warning, TEXT("Running network tests..."));
    FFringeNetwork::RunObserverNetworkTests();
    
    // Check results after tests complete
    UE_LOG(LogTemp, Display, TEXT("Network tests complete"));
}
```

**What it does:**
- Discovers primary and replica servers from configuration
- Makes HTTP GET/POST requests to `/api/health` endpoints
- Logs results: success, latency, status codes
- Records events in observer dashboard

**Configuration (DefaultEngine.ini):**

```ini
[/Script/FringeNetwork.FringeNetworkConfig]
PrimaryServerURL=http://localhost:8080
ReplicaServerURLs=http://replica1.example.com:8080,http://replica2.example.com:8080
TestTimeoutSeconds=10
MaxRetries=3
```

---

### Testing Parallel Realms (Multi-Region)

```cpp
void FFringeNetwork::TestParallelRealms();
```

Validates connectivity and response times across multiple geographic regions.

**Example:**

```cpp
void ANetworkTestGameMode::TestGeographicResilience()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing parallel realms (multi-region)..."));
    FFringeNetwork::TestParallelRealms();
}
```

**What it does:**
- Tests all configured regional servers in parallel
- Measures latency from local to each region
- Identifies regions with high latency or timeouts
- Logs performance metrics and failures

**Configuration (DefaultEngine.ini):**

```ini
[/Script/FringeNetwork.FringeNetworkConfig]
ParallelRealms=(
    (RegionName="US-East", Endpoint="https://us-east.api.example.com"),
    (RegionName="EU-Central", Endpoint="https://eu.api.example.com"),
    (RegionName="Asia-Pacific", Endpoint="https://apac.api.example.com"),
    (RegionName="South-America", Endpoint="https://sa.api.example.com")
)
```

---

### Network Chaos Injection

```cpp
void FFringeNetwork::InjectCortexiphanChaos(float InjectionStrength = 0.5f);
```

Injects network chaos (latency, packet loss) using the Cortexiphan system.

**Example:**

```cpp
void ANetworkChaosTest::TestUnderAdverseConditions()
{
    UE_LOG(LogTemp, Warning, TEXT("Injecting network chaos..."));
    
    // Inject at 50% strength (moderate conditions)
    FFringeNetwork::InjectCortexiphanChaos(0.5f);
    
    // Your game should handle degraded network gracefully
    TestGameplayUnderLatency();
    
    // System automatically blocks unsafe injections (see Cortexiphan docs)
}
```

**What it does:**
- Delegates to `UCortexiphanInjector` for actual chaos
- May be blocked by safety rules (e.g., don't crash during critical sequences)
- Records injection parameters for reproducibility

---

### Observer Network Dashboard

The live dashboard provides real-time monitoring of network events and safety checks.

**Accessing the Dashboard:**

1. Enable ImGui plugin in your project
2. Call `FFringeNetwork::ActivateObserverNetwork()`
3. Run the game
4. Dashboard appears automatically

**Dashboard Display:**

```
┌─ OBSERVER NETWORK — LIVE AUDIT ──────────────────┐
│ REALITY STATUS: STABLE                           │
│──────────────────────────────────────────────────│
│ Session Time: 142.5 seconds                      │
│──────────────────────────────────────────────────│
│ NETWORK EVENTS                                   │
│ ✓ PRIMARY_SERVER_HEALTHY          : 45          │
│ ✓ REPLICA_SYNC_SUCCESS             : 42         │
│ ✗ PACKET_LOSS_DETECTED             : 3          │
│ ✗ HIGH_LATENCY_WARNING             : 7          │
│──────────────────────────────────────────────────│
│ RECENT EVENTS                                    │
│  [  10.2] PRIMARY_SERVER_HEALTHY: Response OK   │
│  [  20.5] REPLICA_SYNC_SUCCESS: Replicated      │
│  [ 105.3] HIGH_LATENCY_WARNING: 250ms detected  │
│  [ 142.1] PRIMARY_SERVER_HEALTHY: Still online  │
└──────────────────────────────────────────────────┘
```

For detailed dashboard usage, see [OBSERVER_NETWORK_GUIDE.md](OBSERVER_NETWORK_GUIDE.md).

---

## Integration Examples

### Example 1: Network Health Check

```cpp
// NetworkHealthCheck.h
UCLASS()
class MYGAME_API UNetworkHealthCheck : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Network")
    void RunHealthCheck();

private:
    void OnHealthCheckComplete();
};
```

```cpp
// NetworkHealthCheck.cpp
#include "NetworkHealthCheck.h"
#include "FringeNetwork/Public/FringeNetwork.h"
#include "FringeNetwork/Public/ObserverNetworkDashboard.h"

void UNetworkHealthCheck::RunHealthCheck()
{
    UE_LOG(LogTemp, Warning, TEXT("🌐 Running network health check..."));
    
    // Initialize observer dashboard
    FFringeNetwork::ActivateObserverNetwork();
    
    // Run tests against configured servers
    FFringeNetwork::RunObserverNetworkTests();
    
    // After completion (async callback would go here)
    OnHealthCheckComplete();
}

void UNetworkHealthCheck::OnHealthCheckComplete()
{
    UE_LOG(LogTemp, Display, TEXT("✅ Network health check complete"));
    UObserverNetworkDashboard::GenerateWebReport();
}
```

---

### Example 2: Regional Failover Test

```cpp
// RegionalFailoverTest.cpp
NEXUS_TEST(FRegionalFailoverTest, "Network.Failover.PrimaryToReplica", ETestPriority::Critical)
{
    // Start with primary server
    UE_LOG(LogTemp, Warning, TEXT("Testing primary region..."));
    FFringeNetwork::RunObserverNetworkTests();
    
    // Simulate primary failure
    UObserverNetworkDashboard::LogSafetyEvent(
        TEXT("PRIMARY_SERVER_DOWN"),
        TEXT("Simulating primary region failure")
    );
    
    // Should automatically failover to replica
    UE_LOG(LogTemp, Warning, TEXT("Testing replica regions..."));
    FFringeNetwork::TestParallelRealms();
    
    UE_LOG(LogTemp, Display, TEXT("✅ Failover test passed"));
    return true;
}
```

---

### Example 3: Chaos Testing with Monitoring

```cpp
// ChaosNetworkTest.cpp
NEXUS_TEST(FChaosNetworkTest, "Network.Chaos.PacketLoss", ETestPriority::High)
{
    // Start monitoring
    FFringeNetwork::ActivateObserverNetwork();
    
    // Inject moderate chaos (50% strength = ~100ms latency, 2% packet loss)
    FFringeNetwork::InjectCortexiphanChaos(0.5f);
    
    // Run actual gameplay under adversity
    SimulatePlayerMovement(30.0f);  // 30 seconds of movement
    SimulateNetworkCommunication();
    
    // Log final status
    UObserverNetworkDashboard::LogSafetyEvent(
        TEXT("CHAOS_TEST_COMPLETE"),
        TEXT("Game remained stable under adverse network conditions")
    );
    
    // Generate report
    UObserverNetworkDashboard::GenerateWebReport();
    
    return true;
}
```

---

## Configuration Guide

### Server Configuration

**DefaultEngine.ini:**

```ini
[/Script/FringeNetwork.FringeNetworkConfig]
; Primary game server
PrimaryServerURL=https://api.mygame.com

; Replica servers for failover
ReplicaServerURLs=https://replica1.mygame.com,https://replica2.mygame.com

; Geographic regions for multi-region testing
ParallelRealms=(
    (RegionName="US-East", Endpoint="https://us-east.api.mygame.com"),
    (RegionName="EU", Endpoint="https://eu.api.mygame.com"),
    (RegionName="APAC", Endpoint="https://apac.api.mygame.com")
)

; Network test parameters
TestEndpoint=/api/health
TestTimeoutSeconds=10
MaxRetries=3
```

### Chaos Parameters

Control the intensity of network chaos injection:

```cpp
// Light chaos (20% strength)
FFringeNetwork::InjectCortexiphanChaos(0.2f);
// Results: ~40ms latency, 1% packet loss

// Moderate chaos (50% strength)
FFringeNetwork::InjectCortexiphanChaos(0.5f);
// Results: ~100ms latency, 2% packet loss

// Heavy chaos (80% strength)
FFringeNetwork::InjectCortexiphanChaos(0.8f);
// Results: ~200ms latency, 5% packet loss
```

---

## Event Logging Convention

Use consistent naming for observer network events:

**Server Health:**
- `PRIMARY_SERVER_HEALTHY` — Primary server responding normally
- `PRIMARY_SERVER_DOWN` — Primary server unreachable
- `REPLICA_SYNC_SUCCESS` — Replica servers synchronized
- `REPLICA_SYNC_FAILED` — Replication failed

**Network Quality:**
- `HIGH_LATENCY_WARNING` — Latency above threshold
- `PACKET_LOSS_DETECTED` — Packet loss observed
- `NETWORK_STABLE` — Network conditions normal

**Chaos Injection:**
- `CHAOS_INJECTION_ACTIVE` — Chaos parameters applied
- `CHAOS_INJECTION_BLOCKED` — Safety rule prevented injection
- `CHAOS_TEST_COMPLETE` — Chaos test finished

Example logging:

```cpp
UObserverNetworkDashboard::LogSafetyEvent(
    TEXT("HIGH_LATENCY_WARNING"),
    FString::Printf(TEXT("Latency: %.0fms, Threshold: %.0fms"), 
        MeasuredLatency, LatencyThreshold)
);
```

---

## Performance Considerations

### Network Request Overhead
- Each test makes ~3-5 HTTP requests
- Typical latency: 50-200ms per request (varies by server)
- Async execution: doesn't block game thread
- Recommended: Run between match phases, not during active gameplay

### Dashboard Rendering
- ImGui dashboard: ~0.5-1ms per frame
- Event log storage: ~100 bytes per event
- Typical usage: 50-200 events per session

### Parallel Realm Testing
- Tests all regions concurrently
- Time = slowest region latency (not sum)
- Typical duration: 5-10 seconds for 4 regions

---

## Troubleshooting

### Observer Network Not Appearing

**Problem:** Dashboard doesn't show during gameplay

**Solutions:**
1. Verify ImGui plugin is enabled
2. Ensure `ActivateObserverNetwork()` was called
3. Check `GameViewport` exists
4. Try pressing `Shift+F1` to toggle UI visibility

### Network Tests Timeout

**Problem:** `RunObserverNetworkTests()` hangs

**Solutions:**
1. Check server URLs in `DefaultEngine.ini`
2. Verify servers are actually running
3. Increase `TestTimeoutSeconds` value
4. Check firewall/network connectivity

### Chaos Injection Gets Blocked

**Problem:** `InjectCortexiphanChaos()` doesn't apply changes

**Solutions:**
1. Check Cortexiphan safety rules (see Cortexiphan docs)
2. Log safety events to understand why: `LogSafetyEvent("CHAOS_INJECTION_BLOCKED", ...)`
3. Reduce injection strength (0.2f instead of 0.8f)
4. Try different injection time (not during critical sequence)

---

## See Also

- [OBSERVER_NETWORK_GUIDE.md](OBSERVER_NETWORK_GUIDE.md) — Dashboard and event logging
- [Cortexiphan Chaos Injection Guide](./CORTEXIPHAN_GUIDE.md)
- [Nexus Core Framework](./NEXUS_GUIDE.md)
- [API Testing with PalantírRequest](./API_TESTING.md)
