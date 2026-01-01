# NexusQA — Modular Testing Framework for Unreal Engine 5

<div align="center">

![NexusQA Banner](.github/banner_animated.gif)

![Safety Checks](https://github.com/calionestevar/NexusQA/actions/workflows/stone-table.yml/badge.svg)
![LCARS Demo](https://github.com/calionestevar/NexusQA/actions/workflows/lamppost-beacon.yml/badge.svg)

</div>

A clean, extensible QA automation framework for UE5 featuring parallel execution, distributed tracing, chaos engineering, and compliance testing. Built to demonstrate modern C++ patterns and game industry best practices.

**Codename: Project Asgard** — A fusion of sci-fi and fantasy universes (Stargate, Star Trek, LOTR, Fringe, Harry Potter, Narnia) powering real-world QA solutions.

---

## 🎯 TL;DR — What This Is

**For Recruiters:** A portfolio project showcasing production-quality C++ for game QA automation. Demonstrates parallel algorithms, distributed tracing, chaos testing, and compliance validation — all using Unreal Engine 5 APIs.

**Key Skills Demonstrated:**
- ✅ Modern C++ (RAII, thread-local storage, async patterns, smart pointers)
- ✅ Unreal Engine 5 (automation framework, memory profiling, HTTP module)
- ✅ Game Industry Patterns (distributed tracing like Sentry, API testing, performance monitoring)
- ✅ Clean Architecture (modular design, dependency injection, fluent APIs)

**What It Does:** Runs game tests in parallel, monitors performance (FPS/memory), simulates network chaos, validates compliance (COPPA/GDPR), and generates Star Trek-themed HTML reports.

**Tech Stack:** C++17, Unreal Engine 5.7, HTTP/JSON APIs, GitHub Actions CI/CD

---

<div align="center">

**📋 For Recruiters:** [View Portfolio Summary](PORTFOLIO.md) — Skills, achievements, and code highlights  
**📚 For Developers:** Continue reading below for technical documentation

</div>

---

## 🎯 Core Modules

| Module | Purpose | Key Features |
|--------|---------|--------------|
| **Nexus** | Test orchestration & parallel execution | Thread pool execution, fail-fast critical tests, test discovery |
| **Palantír** | Distributed tracing & API testing | Thread-local trace context, REST/GraphQL validation, fluent assertions |
| **LCARSBridge** | Star Trek-themed HTML/JSON/XML reports | LCARS-styled dashboards, artifact generation, multi-format export |
| **FringeNetwork** | Network chaos engineering | Lag injection, packet loss simulation, disconnect testing |
| **StargateStress** | Load testing & bot simulation | Concurrent user simulation, behavior patterns, safety system validation |
| **ArgusLens** | Performance monitoring | Real-time FPS tracking, memory profiling, hitch detection |
| **Protego** | Compliance & accessibility | COPPA/GDPR/DSA checks, color-blind support, subtitle validation |
| **Legacy** | UE5 AutomationTest integration | Commandlet bridge to native automation framework |

---

## 🚀 Quick Start

### Windows (Recommended)
```powershell
git clone https://github.com/calionestevar/NexusQA
cd NexusQA
.\Scripts\Engage.bat
```

### Linux/macOS
```bash
git clone https://github.com/calionestevar/NexusQA
cd NexusQA
./Scripts/RideOut.sh
```

---

## 🔌 Integration as Reusable Framework

### Option 1: Git Submodule (Recommended)
Add NexusQA as a plugin to your UE5 project:

```bash
cd YourGameProject
git submodule add https://github.com/calionestevar/NexusQA.git Plugins/NexusQA
git commit -m "Add NexusQA testing framework"
```

Then regenerate your project files and rebuild.

**Updating to latest framework:**
```bash
git submodule update --remote Plugins/NexusQA
```

### Option 2: Manual Copy
Copy the entire `Plugins/NexusQA/` folder to your project's `Plugins/` directory and rebuild.

---

### Generate Demo LCARS Report
```powershell
.\Scripts\Generate-LCARSReport.ps1
```
Opens `TestReports/LCARS_Demo_Report.html` in your browser — screenshot-ready!

---

## 📋 What This Framework Produces

### Test Artifacts (Saved to `Saved/NexusReports/`)
- **LCARS_Report_\<timestamp\>.html** — Star Trek-themed HTML dashboard with test results
- **nexus-results.xml** — JUnit-style XML for CI/CD integration
- **test_\<name\>.log** — Per-test execution logs (linked from JUnit output)
- **performance_\<test\>.json** — FPS, memory, and hitch metrics
- **accessibility_\<test\>.json** — Compliance check results

### CI/CD Integration
GitHub Actions workflows:
- **stone-table.yml** ✅ Active — Safety pattern detection (dangerous code patterns)
- **lamppost-beacon.yml** ✅ Active — Template generation & HTML report generation
- **aslan-vigil.yml** — Currently disabled (can be re-enabled for test suite execution)
- **cair-paravel.yml** — Currently disabled (can be re-enabled for dependency scanning)

---

## 🏗️ Architecture Highlights

### Parallel Execution
Uses Unreal's `Async(EAsyncExecution::ThreadPool)` for fast test execution without process overhead.

```cpp
TArray<TFuture<bool>> Futures;
for (FNexusTest* Test : DiscoveredTests) {
    Futures.Add(Async(EAsyncExecution::ThreadPool, [Test]() {
        return Test->Execute();
    }));
}
```

### Distributed Tracing
Thread-local trace context with automatic ID propagation:

```cpp
FPalantirTraceGuard TraceGuard; // RAII guard
FString TraceID = FPalantirTrace::GetCurrentTraceID();
// Trace ID automatically flows through nested calls
```

### Fluent API Testing
Readable, chainable assertions for API testing:

```cpp
PalantirRequest()
    .SetURL("https://api.example.com/users")
    .SetMethod(EHttpMethod::GET)
    .SetTimeout(5.0f)
    .ExpectStatus(200)
    .ExpectHeader("Content-Type", "application/json")
    .ExpectJsonPath("$.users[0].name", "Alice")
    .Send();
```

### Real Performance Monitoring
ArgusLens samples actual UE5 metrics:

```cpp
float FPS = 1.0f / FApp::GetDeltaTime();
FPlatformMemoryStats MemStats;
FPlatformMemory::GetStatsForMallocProfiler(MemStats);
uint64 UsedMemoryMB = MemStats.UsedPhysical / (1024 * 1024);
```

---

## 📚 Documentation

### Core Framework Guides
- **[PORTFOLIO.md](PORTFOLIO.md)** — 🎯 Technical showcase for recruiters
- **[Docs/NEXUS_GUIDE.md](Docs/NEXUS_GUIDE.md)** — Test framework core: discovery, execution, reporting
- **[OBSERVER_NETWORK_GUIDE.md](OBSERVER_NETWORK_GUIDE.md)** — Real-time dashboard and safety event logging
- **[Docs/ARGUSLENS_GUIDE.md](Docs/ARGUSLENS_GUIDE.md)** — Performance monitoring: FPS, memory, hitches
- **[Docs/FRINGENETWORK_GUIDE.md](Docs/FRINGENETWORK_GUIDE.md)** — Network chaos: lag injection, packet loss, failover
- **[Docs/STARGATESTRESS_GUIDE.md](Docs/STARGATESTRESS_GUIDE.md)** — Load testing: bot simulation, stress validation
- **[Docs/PROTEGO_GUIDE.md](Docs/PROTEGO_GUIDE.md)** — Compliance: COPPA/GDPR/DSA, accessibility testing

### Advanced Topics
- **[Docs/API_TESTING.md](Docs/API_TESTING.md)** — REST/GraphQL testing patterns
- **[Docs/PALANTIR.md](Docs/PALANTIR.md)** — Distributed tracing deep-dive
- **[Docs/LCARS_PROVIDERS.md](Docs/LCARS_PROVIDERS.md)** — Report generation system
- **[Docs/modules.md](Docs/modules.md)** — Module architecture reference

### Development
- **[BUILD.md](BUILD.md)** — Build instructions and dependencies
- **[CONTRIBUTING.md](CONTRIBUTING.md)** — Code standards and patterns
- **[Docs/GAME_INDUSTRY_INTEGRATIONS.md](Docs/GAME_INDUSTRY_INTEGRATIONS.md)** — Sentry, PlayFab, GameLift

---

## 🛡️ Safety & Security

### Tok'Ra Pre-Commit Hook
Prevents commits containing dangerous patterns:

```bash
# Install the git hook
.\Scripts\install-tokra.bat  # Windows
./Scripts/install-tokra.sh   # Unix
```

Blocks:
- `MakeShareable(this)` — dangerous shared pointer patterns
- Hardcoded secrets (API keys, tokens, passwords)
- Suspicious patterns (shell injection, path traversal)

### CI Safety Checks
- **stone-table.yml** — Grep-based pattern detection
- **cair-paravel.yml** — Dependency vulnerability scanning

---

## 🎨 LCARS Report Preview

The framework generates beautiful Star Trek-themed reports featuring:

- **Executive Summary** — Pass/fail statistics, execution time
- **API Metrics** — Request counts, status codes, response times
- **Performance Data** — FPS graphs, memory usage, hitch detection
- **Test Details** — Individual results with trace IDs and artifacts

**Visual Style:**
- Dark blue gradient background (`#000033` → `#001155`)
- LCARS color scheme (orange `#ff9900`, yellow `#ffcc00`, cyan `#00ccff`)
- Monospace "Courier New" font for authenticity
- Responsive grid layouts for all screen sizes

---

## 🧪 Example Test

```cpp
IMPLEMENT_NEXUS_TEST(FMyGameplayTest)
{
    FPalantirTraceGuard TraceGuard; // Auto trace context
    
    // Performance monitoring
    UArgusLens* Lens = NewObject<UArgusLens>();
    Lens->StartPerformanceMonitoring();
    
    // Test logic
    bool bGameplayPassed = RunGameplayScenario();
    
    // Export results
    Lens->StopPerformanceMonitoring();
    Lens->ExportPerformanceArtifact("MyGameplayTest");
    
    return bGameplayPassed && Lens->DidPassPerformanceGates();
}
```

---

## 📦 Module Dependencies

```
Nexus
├── Core (Engine, CoreUObject)
├── Palantír (HTTP, Json, JsonUtilities)
└── LCARSBridge (Json)

FringeNetwork → Nexus, Sockets
StargateStress → Nexus, AIModule
ArgusLens → Nexus, Engine
Protego → Nexus, Engine
Legacy → Nexus, AutomationController
```

---

## 🗺️ Roadmap

**Near Term:**
- [ ] Blueprint test coverage support
- [ ] Integration with Unreal Insights for deeper profiling
- [ ] Multiplayer session regression testing
- [ ] EOS (Epic Online Services) integration testing
- [ ] Enhanced chaos injection (CPU throttling, memory pressure)
- [ ] Visual regression testing for UI
- [ ] Smoke test CI workflow for quick validation

**Under Consideration:**
- [ ] Automated test generation from Blueprint graphs
- [ ] Integration with GameLift for cloud testing
- [ ] Performance baseline tracking and regression detection
- [ ] Separate Unity QA framework project (future exploration)

---

## 🤝 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Code style guidelines
- Pull request process
- Testing requirements
- Documentation standards

---

## 📝 License

MIT License — See [LICENSE](LICENSE) for details.

---

## 🌟 Credits

**Inspired by:**
- Star Trek (LCARS interface design)
- Stargate SG-1 (Tok'Ra, Goa'uld, Asgard references)
- Lord of the Rings (Palantír seeing stones)
- Fringe (Cortexiphan, parallel universes)
- Harry Potter (Hogwarts, Protego shield charm)
- Chronicles of Narnia (Aslan, Cair Paravel)

Built to showcase clean architecture, modern C++ patterns, and game industry integration expertise.
