# Architecture: NexusQA Design

This document explains the high-level design philosophy, module interdependencies, and architectural decisions behind the NexusQA framework.

## Design Philosophy

**Goal:** Create a **test framework that mirrors how games are built** — modular, parallel, resilient to chaos, and observable.

**Key Principles:**
1. **Modularity** — Each testing concern (network, performance, compliance) is independent
2. **Parallelism** — Tests run in parallel workers; failures don't block others
3. **Observability** — Real-time dashboards, rich artifacts, and detailed logs
4. **Realism** — Inject chaos, simulate multiplayer, measure real performance
5. **Flexibility** — Swap components; add custom test runners and reporters

---

## Layered Architecture

```
┌─────────────────────────────────────────────────────┐
│              Test Harnesses                         │
│  (Scripts/Engage.ps1, RideOut.sh, CI workflows)     │
└──────────────────┬──────────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────────┐
│          NexusDemo (Target/Entry Point)             │
│  ├─ Loads all modules                              │
│  ├─ Boots Nexus orchestration engine                │
│  └─ Runs tests or commandlets                       │
└──────────────────┬──────────────────────────────────┘
                   │
    ┌──────────────┼──────────────┐
    │              │              │
┌───▼────────┐ ┌──▼────────┐ ┌──▼─────────┐
│   Nexus    │ │ Utilities │ │  Legacy    │
│  (Core)    │ │(Shared    │ │(Asgard     │
│            │ │ Config)   │ │Commandlet) │
└───┬────────┘ └──┬────────┘ └──┬─────────┘
    │             │             │
    └─────────────┼─────────────┘
                  │
    ┌─────────────┼─────────────────────────────┐
    │             │             │                │
┌───┴──────┐ ┌──┴──────────┐  ┌────┴───┐  ┌───────┴─────┐
│ Protego  │ │FringeNet  │  │ArgusLens  │StargateStrs│
│(Compliance)│(Network)     │(Perf)    │(Load Test) │
└──────────┘ └────────────┘  └──────────┘└─────────────┘
```

---

## Core Modules

### **Nexus** (Orchestration Engine)
**Purpose:** Discover, schedule, and execute tests in parallel with rich reporting.

**Responsibilities:**
- Test discovery (scan for `NEXUS_TEST` macros)
- Parallel worker pool management
- Critical test fail-fast logic
- Result aggregation & reporting
- PalantírObserver integration (live dashboards)
- Test context provisioning (world, game state, player controller access)
- Optional performance metrics collection from ArgusLens

**Key Classes:**
- `UNexusCore` — Main orchestration engine
- `UNexusTest` — Base class for all tests with skip/retry support
- `FNexusTestContext` — Provides game world access, actor spawning, cleanup, and performance metrics
- `FTestPerformanceMetrics` — ArgusLens integration for FPS/memory/hitch assertions
- `UPalantirObserver` — In-memory result map & live reporting
- `ILCARSResultsProvider` — Pluggable report backend

**Test Macros:**
- `NEXUS_TEST()` — Standard unit/integration test
- `NEXUS_TEST_GAMETHREAD()` — Game-thread test with world context
- `NEXUS_PERF_TEST()` — Performance test with ArgusLens metrics
- Built-in support for `bSkip` flag and `MaxRetries` with exponential backoff

**Entry Point:** `Nexus.Execute` command in `Config/DefaultEngine.ini`

---

### **Utilities** (Shared Configuration)
**Purpose:** Centralized, config-driven constants used across all modules.

**Provides:**
- `UNerdyConstants` — Gameplay thresholds, AI keywords, compliance flags
- Standard report output paths
- Framework-wide tuning knobs

**Why It Exists:** Prevents code duplication; allows non-programmers to tweak behavior via `.ini` files.

---

### **LCARSBridge** (Report Generation & Presentation)
**Purpose:** Transform observability data into consumable reports for humans and CI systems.

**Provides:**
- `FLCARSHTMLGenerator` — Star Trek-themed HTML dashboard generation
- `ILCARSProvider` — Pluggable provider interface for multiple data sources
- `FPalantirLCARSProvider` — Connects to Palantír observability data
- `FAutomationTestLCARSProvider` — Connects to UE Automation Framework
- `LCARSReporter` — Legacy compatibility layer for report export

**Why "Bridge":** In Star Trek, The Bridge is the command center where all ship systems report status — similarly, LCARSBridge receives data from all testing modules (Palantír, ArgusLens, Protego) and presents unified reports.

**Artifacts:** HTML dashboards, JSON exports, JUnit XML for CI integration

---

## Feature Modules

### **Protego** (Compliance & Accessibility)
**Purpose:** Audit games for legal/ethical compliance and accessibility standards.

**Modules:**
- `UDefenseAgainstTheDarkArts` — Minor-protection compliance checks (COPPA, GDPR, DSA)
- `UTransfiguration` — Accessibility audit suite (color-blind, subtitles, input remapping, contrast)
- **Data:** JSON rule files under `Private/ComplianceRules/`

**High-Impact for Recruiters:** Shows understanding of modern gaming regulations & inclusive design.

---

### **FringeNetwork** (Network & Multiplayer Testing)
**Purpose:** Inject realism into networked gameplay testing.

**Modules:**
- `UCortexiphanInjector` — Network chaos (latency, packet loss, bandwidth throttling)
- `UObserverNetworkDashboard` — Real-time network event monitoring
- `UBishopBridge` — In-process client replication harness
- `UFringeNetwork` — Parallel world simulation for stress testing

**Why Valuable:** Most game failures occur under bad network conditions; this framework makes them reproducible and measurable.

---

### **ArgusLens** (Performance Monitoring)
**Purpose:** Continuous FPS, memory, and hitch tracking with artifact export.

**Modules:**
- `UArgusLens` — Periodic sampling of frame time, memory, hitches
- **Artifacts:** JSON metrics + HTML dashboards

**Why Valuable:** Performance is non-negotiable in games; this makes it a first-class test concern.

---

### **StargateStress** (Synthetic Chat & NPC Behavior)
**Purpose:** Simulate large numbers of players or NPCs with realistic behavior.

**Modules:**
- `UReplicatorSwarm` — Behavior profile orchestrator
- **Data:** BehaviorProfiles/ for configurable NPC actions

**Why Valuable:** Multiplayer games need stress-tested concurrent behavior; this framework makes it scriptable.

---

### **Legacy** (Historical Tools)
**Purpose:** Backward compatibility with older Asgard commandlet-based test runner.

**Modules:**
- `AAsgardCore` — Legacy test harness
- `AAsgardCommandlet` — Engine-native commandlet entry point
- `UPalantirAnalyzer` — Bias/fairness audit
- `UPalantirCapture` — Automatic screenshot capture

**Why It Exists:** Some projects still use Unreal's AutomationTest framework; Legacy bridges that gap.

---

## Data Flow & Integration Points

```
┌────────────────────────────────────────────────────────┐
│ Test Discovery (Nexus scans for NEXUS_TEST macros)     │
└──────────────────┬─────────────────────────────────────┘
                   │
                   ▼
┌────────────────────────────────────────────────────────┐
│ Test Execution (Parallel workers in Nexus)             │
│  ├─ Each test is independent                           │
│  ├─ Can spawn Protego, Chaos, ArgusLens checks         │
│  └─ Results fed to PalantírObserver                     │
└──────────────────┬─────────────────────────────────────┘
                   │
                   ▼
┌────────────────────────────────────────────────────────┐
│ Reporting (LCARS Provider)                             │
│  ├─ PalantírInMemoryProvider reads PalantírObserver    │
│  ├─ Serializes to JSON/XML/HTML                        │
│  └─ Outputs to Saved/NexusReports/                     │
└────────────────────────────────────────────────────────┘
```

---

## Module Dependencies

**Golden Rule:** Dependencies flow **downward** only. No circular deps.

```
NexusDemo
    ├─ Nexus (all tests depend on this)
    │   └─ Utilities (config)
    │       └─ (no deps)
    │
    ├─ Protego → Utilities
    ├─ Chaos → Utilities
    ├─ ArgusLens → Utilities
    ├─ SwarmOfTheDead → Utilities
    └─ Legacy → Utilities (optional)
```

**Adding a New Module?** Ensure it only depends on modules *below* it.

---

## Test Organization

Tests live **alongside the code they test**:

```
Source/ModuleName/Private/Tests/
  ├─ FeatureA.test.cpp
  ├─ FeatureB.test.cpp
  └─ Integration/
     └─ CrossModule.test.cpp
```

**Why This Pattern?**
- Developers naturally update tests when changing code
- Easy to discover relevant tests
- Mirrors enterprise/AAA practices (Google, Meta, major game studios)
- Scales as the framework grows

---

## Reporting & Artifacts

All test output lands in `Saved/NexusReports/`:

| Artifact | Purpose |
|----------|---------|
| `LCARSReport.json` | Structured test results (compatible with CI tools) |
| `LCARS_Report_*.html` | Human-friendly Starfleet-themed dashboard |
| `nexus-results.xml` | JUnit-compatible XML (GitHub Actions, Jenkins, etc.) |
| `ArgusLensPerformance.json` | Detailed performance metrics |
| `TransfigurationReport.json` | Accessibility audit results |
| `FringeNetwork_events.json` | Network event logs |

**Why Multiple Formats?**
- JSON for parsing/analysis
- HTML for quick visual review
- XML for CI tool integration
- Specialized JSON for each domain (perf, compliance, etc.)

---

## Extension Points

### Adding Custom Tests
```cpp
// In Source/MyModule/Private/Tests/
#include "NexusTest.h"

NEXUS_TEST(FMyTest, "MyModule.Feature.Scenario", ETestPriority::Normal)
{
    // Your test logic
    return true; // or false
}
```

### Adding Custom Modules
1. Create `Source/NewModule/` folder
2. Add `NewModule.Build.cs` with proper dependencies
3. Register in `NexusDemo.uproject`
4. Implement test interfaces if needed

### Plugging in Custom Reporters
1. Implement `ILCARSResultsProvider`
2. Register in `Config/DefaultEngine.ini`:
   ```
   [/Script/Nexus.Palantir]
   CustomProviderClass=/Game/Path/To/MyProvider
   ```

---

## Design Decisions & Trade-offs

| Decision | Rationale | Trade-off |
|----------|-----------|-----------|
| **Tests in modules, not separate folder** | Encourages TDD; easy to find/maintain | Slightly larger module footprint |
| **NEXUS_TEST macro instead of UE AutomationTest** | Cleaner syntax; better for large test suites | Less compatible with legacy Unreal tools (mitigated by Legacy module) |
| **JSON + HTML reporting** | Both structured (for CI) and human-readable | Multiple format maintenance |
| **In-process chaos simulation** | Fast iteration; no network setup | Can't test true network stack edge cases |
| **Pluggable LCARS providers** | Framework remains flexible; supports multiple backends | Added complexity for simple use cases |

---

## Performance Considerations

- **Parallel execution:** Tests run in worker threads; framework automatically isolates state
- **Memory:** Each worker gets its own game world context; clean teardown between tests
- **Networking:** Chaos simulator uses lightweight event injection; doesn't stress actual network
- **Reporting:** JSON serialization is lazy; HTML generation deferred until end of suite

---

## Module Reference

This section provides detailed documentation of each module in the NexusQA framework, including purpose, key classes, dependencies, and artifacts.

### Core Framework

#### **Nexus** (Core Orchestration)
- **Purpose:** Core QA framework orchestration, target integration, and CI/CD hooks.
- **Key Classes:** 
  - `UNexusCore`: Test orchestration and execution
  - `UNexusTest`: Base test class with automatic tracing
  - **Palantír Subsystem:**
    - `FPalantirOracle`: In-process result aggregation and live dashboard
    - `FPalantirTrace`: Distributed tracing with correlation IDs and breadcrumbs
    - `FPalantirVision`: Rich assertions with context capture
    - `FPalantirRequest`: REST & GraphQL API testing with automatic tracing
- **Features:**
  - Automatic trace ID injection (`UE_LOG_TRACE` macro)
  - Breadcrumb timeline tracking (`PALANTIR_BREADCRUMB` macro)
  - Enhanced assertion macros (`NEXUS_ASSERT_GT`, `NEXUS_ASSERT_LT`, etc.)
  - REST & GraphQL request validation (fluent API, JSONPath, retry logic)
  - JSON export for game industry tools (Sentry, PlayFab, GameAnalytics, Unreal Insights)
  - Result aggregation for LCARS reporter
- **Dependencies:** Core, CoreUObject, Engine, Projects, InputCore, Slate, SlateCore, ImGui, HTTP, Json, JsonUtilities
- **Artifacts:** Test results, JSON reports, CI logs, traces.jsonl
- **Usage:** Base module for all tests; Palantír provides automatic tracing, rich error context, and API contract validation.

#### **NexusDemo** (Demo Target)
- **Purpose:** Demo/example project showing Nexus framework capabilities.
- **Key Classes:** `UNexusDemo`
- **Dependencies:** Nexus (and all Nexus deps)
- **Artifacts:** Demo reports, metrics
- **Usage:** Run the demo build target to see framework in action.

#### **Utilities** (Shared Configuration)
- **Purpose:** Centralized, config-driven constants used across all modules.
- **Key Classes:** `UNerdyConstants`
- **Contents:** Gameplay thresholds, AI keywords, compliance flags, reporting paths
- **Dependencies:** Core, CoreUObject, Engine, Json, JsonUtilities
- **Usage:** Import `UNerdyConstants` in any module needing framework-wide configuration.

#### **LCARSBridge** (Report Presentation)
- **Purpose:** Transform test results and observability data into human-readable and CI-compatible reports.
- **Key Classes:**
  - `FLCARSHTMLGenerator`: Starfleet-themed HTML dashboard generator
  - `ILCARSProvider`: Pluggable provider interface
  - `FPalantirLCARSProvider`: Palantír data source adapter
  - `FAutomationTestLCARSProvider`: UE Automation Framework adapter
  - `LCARSReporter`: Legacy report export compatibility
- **Contents:** HTML templates, provider implementations, report formatters
- **Dependencies:** Core, CoreUObject, Engine, Json, JsonUtilities, HTTP (inherits from Nexus module)
- **Artifacts:** `LCARS_Report_*.html`, `LCARSReport.json`, JUnit XML
- **Usage:** Automatically invoked by Palantír at test suite completion; can be called directly via `LCARSReporter::ExportResultsToLCARS()`

### Feature Modules

#### **Protego** (Compliance & Accessibility)
- **Purpose:** Compliance auditing and accessibility testing.
- **Key Classes:** `UDefenseAgainstTheDarkArts` (compliance), `UTransfiguration` (accessibility)
- **Contents:**
  - Compliance rule JSON files (COPPA, GDPR, DSA)
  - Accessibility audit suite (color-blind simulation, subtitles, input remapping, contrast checks)
- **Dependencies:** Core, CoreUObject, Engine, Json, JsonUtilities
- **Artifacts:** `Saved/Accessibility/TransfigurationReport.json`, compliance audit logs
- **Usage:** Call `UTransfiguration::RunAccessibilityAudit()` and compliance checks from test harnesses.

#### **FringeNetwork** (Network & Multiplayer)
- **Purpose:** Network chaos injection, multiplayer testing, and advanced network simulation.
- **Key Classes:**
  - `UCortexiphanInjector`: Network latency/packet loss/bandwidth simulation
  - `UObserverNetworkDashboard`: Network metrics monitoring
  - `UBishopBridge`: In-process client replication harness for multiplayer testing
  - `UFringeNetwork`: Parallel world simulation for stress testing
- **Dependencies:** Core, CoreUObject, Engine, Json, JsonUtilities, Projects, InputCore, Slate, SlateCore
- **Artifacts:** `Saved/NexusReports/FringeNetwork_*.json`, network event logs
- **Usage:** Inject chaos into running tests; monitor network behavior; simulate multiplayer scenarios.

#### **ArgusLens** (Performance Monitoring)
- **Purpose:** Performance monitoring and metrics collection.
- **Key Classes:** `UArgusLens`
- **Features:** FPS tracking, memory profiling, hitch detection, performance thresholds, artifact export
- **Dependencies:** Core, CoreUObject, Engine, Json, JsonUtilities
- **Artifacts:** `Saved/NexusReports/ArgusLensPerformance.json`, HTML dashboards
- **Usage:** Call `UArgusLens::StartPerformanceMonitoring()` before gameplay; export metrics post-run.

#### **StargateStress** (NPC & Chat Simulation)
- **Purpose:** Synthetic chat simulation and NPC behavior generation.
- **Key Classes:** `UReplicatorSwarm`
- **Contents:** Behavior profiles for AI chat and NPC simulation
- **Dependencies:** Core, CoreUObject, Engine, Json, JsonUtilities, Projects, InputCore, Slate, SlateCore
- **Artifacts:** Synthetic conversation logs, NPC behavior metrics
- **Usage:** Simulate player-NPC interactions at scale.

### Support Modules

#### **Legacy** (Historical Tools)
- **Purpose:** Backward compatibility with older Asgard commandlet-based test runner.
- **Key Classes:**
  - `AAsgardCore`: Test harness base
  - `AAsgardCommandlet`: Batch test runner
  - `UPalantirAnalyzer`: Bias/fairness auditing
  - `UPalantirCapture`: Session recording
  - `UGoauldGatekeeper`: Access control
- **Dependencies:** Core, CoreUObject, Engine, Json, JsonUtilities, Projects, InputCore, Slate, SlateCore
- **Usage:** Optional; used for backward compatibility with older test harnesses.

### Dependency Graph

```
Nexus (core + Palantír tracing/assertions)
  └─ NexusDemo (demo target)
     └─ All feature modules (FringeNetwork, ArgusLens, Protego, StargateStress)

Utilities (shared config)
  ├─ Nexus
  ├─ Legacy
  ├─ Protego
  ├─ FringeNetwork
  ├─ ArgusLens
  └─ StargateStress

Legacy (historical tools)
  └─ Optional; used for backward compatibility
```

### Build & Compilation

All modules are declared in `NexusDemo.uproject`:
```json
"Modules": [
  { "Name": "NexusDemo", "Type": "Runtime", "LoadingPhase": "Default" },
  { "Name": "Nexus", "Type": "Runtime", "LoadingPhase": "Default" },
  { "Name": "Utilities", "Type": "Runtime", "LoadingPhase": "Default" },
  { "Name": "Protego", "Type": "Runtime", "LoadingPhase": "Default" },
  { "Name": "FringeNetwork", "Type": "Runtime", "LoadingPhase": "Default" },
  { "Name": "ArgusLens", "Type": "Runtime", "LoadingPhase": "Default" },
  { "Name": "StargateStress", "Type": "Runtime", "LoadingPhase": "Default" },
  { "Name": "Legacy", "Type": "Runtime", "LoadingPhase": "Default" },
  { "Name": "Toolkit", "Type": "Runtime", "LoadingPhase": "Default" }
]
```

Each module has a corresponding `ModuleName.Build.cs` defining dependencies. To add a new module:
1. Create `Source/ModuleName/ModuleName.Build.cs`
2. Add module entry to `.uproject`
3. Create `Public/` and `Private/` folders with headers/cpp
4. Regenerate project files and rebuild.

### Artifact Organization

All test artifacts export to `Saved/NexusReports/<ModuleName>/`:
- `ArgusLens_*.json` — Performance metrics
- `Protego_*.json` — Compliance and accessibility reports
- `FringeNetwork_*.json` — Network event logs
- `*_report.html` — Human-readable reports

### Adding New Features

1. **New Test Framework:** Create a new module (e.g., `Source/Rendering/`) with its own Build.cs and register in `.uproject`.
2. **Shared Types:** Add to `Utilities/Public/` so all modules can depend on it.
3. **Isolated Tool:** Create a standalone module under `Source/` with minimal external dependencies.

---

## Future Evolution

Potential high-impact additions:
- **WebSocket API testing** — Validate game servers/backend
- **Visual regression testing** — Compare frame-perfect renders
- **Cross-platform input validation** — Keyboard, gamepad, touch mapping
- **Multiplayer state validator** — Assert replication correctness
- **Web-based dashboard** — Real-time results via HTTP API

---

## References

- [../README.md](../README.md) — Project overview
- [../CONTRIBUTING.md](../CONTRIBUTING.md) — Code standards & contribution process
- [API_TESTING.md](API_TESTING.md) — REST & GraphQL API testing guide
- [LCARS_PROVIDERS.md](LCARS_PROVIDERS.md) — Reporting configuration & providers
- [PALANTIR.md](PALANTIR.md) — Distributed tracing & assertions
- [GAME_INDUSTRY_INTEGRATIONS.md](GAME_INDUSTRY_INTEGRATIONS.md) — Tool integrations
