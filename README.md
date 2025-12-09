# Nerdy QA Toolkit — UE5 Automation Framework

[![Loom Demo (15s)]**Coming Soon!**

**Zero-setup UE5 QA automation — 45-second full-suite runtime**

Codename: **Project Asgard** (Stargate's ancient allies + LOTR's Valinor + Star Trek's Asgard-class ships)

## Features
- **Asgard Commandlet** — one-click headless test run + JSON export  
- **GondorCallsForAid** — CI trigger script (Windows/Linux/Mac)  
- **LCARS Reporter** — Starfleet-style HTML test reports  
- **Palantír Capture** — automatic failure screenshots  
- **Tok'Ra Symbiote Blockers** — red regression tests for UE5 gremlins  

## Quick Start
# Nerdy QA Toolkit — UE5 Automation Framework

![CI](https://github.com/calionestevar/NexusQA/actions/workflows/aslan-vigil.yml/badge.svg)

Zero-setup UE5 QA automation — showcase of orchestration, reporting, and CI-friendly artifacts.

Codename: **Project Asgard** (a playful fusion of your favorite universes: Stargate, LOTR, Star Trek, and more)

## Features
- **Nexus** — modern test orchestration (discovery, parallel workers, fail-fast critical tests)
- **Palantír** — distributed tracing, API testing, rich assertions
  - Automatic trace ID propagation (Sentry/PlayFab/GameAnalytics/Unreal Insights integration)
  - REST & GraphQL request validation with fluent API
  - Response assertions (status codes, headers, JSON paths)
  - Retry logic with exponential backoff
- **Asgard Commandlet (Legacy)** — engine-native commandlet path using Unreal's AutomationTest framework
- **LCARS Reporter** — Starfleet-style test summary with per-test artifacts
- **Valkyrie** — optional auto-upgrader workflow for dependency maintenance (vcpkg/Conan/NuGet)

## Quick Start
Windows (recommended):

```powershell
git clone https://github.com/calionestevar/NexusQA
cd nerdy-qa-toolkit
\Scripts\Engage.bat         # invokes PowerShell harness: Scripts/Engage.ps1
```

Linux/macOS:

```bash
git clone https://github.com/calionestevar/NexusQA
cd NexusQA
./Scripts/RideOut.sh
```

## What CI produces
- `Saved/NexusReports/LCARS_Report_<timestamp>.html` — human-friendly HTML report
- `Saved/NexusReports/nexus-results.xml` — JUnit-style XML for CI dashboards
- `Saved/NexusReports/test_<name>.log` — per-test artifact/log (linked from JUnit `<system-out>`)

See `Docs/TestingFramework.md` for more detail about running tests and migrating legacy Asgard commandlets to Nexus.

## 📚 Documentation

**Quick Links:**
- **[PORTFOLIO.md](PORTFOLIO.md)** — 🎯 Technical showcase (architecture, code examples, achievements)
- **[docs/API_TESTING.md](docs/API_TESTING.md)** — REST/GraphQL testing guide  
- **[docs/LCARS_PROVIDERS.md](docs/LCARS_PROVIDERS.md)** — HTML reporter and provider system
- **[docs/PALANTIR.md](docs/PALANTIR.md)** — Distributed tracing, assertions, result aggregation
- **[docs/GAME_INDUSTRY_INTEGRATIONS.md](docs/GAME_INDUSTRY_INTEGRATIONS.md)** — Sentry, PlayFab, GameLift
- **[docs/modules.md](docs/modules.md)** — Module architecture
- **[BUILD.md](BUILD.md)** — Build instructions
- **[CONTRIBUTING.md](CONTRIBUTING.md)** — Code standards

## 📸 Screenshots & Artifacts

**Visual showcase of framework capabilities:**

### LCARS HTML Dashboard ✅
The enhanced LCARS reporter generates beautiful Starfleet-themed HTML dashboards with:
- **Executive Summary** - Total tests, pass/fail counts, pass rate percentage
- **API Metrics** - Request/response statistics, status code distribution, endpoint performance
- **Performance Metrics** - FPS tracking, memory usage, hitch detection
- **Test Details** - Individual test results with trace IDs and artifacts

**Generate Demo Report:**
```powershell
.\Scripts\Generate-LCARSReport.ps1
```
Opens `TestReports/LCARS_Demo_Report.html` in your browser - screenshot-ready!

### API Testing with Trace Correlation
*Coming soon: Screenshot showing trace ID flow from Unreal → PlayFab → Sentry*

### Performance Monitoring Dashboard
*Coming soon: ArgusLens FPS/memory graphs with hitch detection*

### Distributed Tracing Timeline
*Coming soon: JSON export showing breadcrumb timeline across multiple services*

See **[SAMPLE_ARTIFACTS.md](Docs/SAMPLE_ARTIFACTS.md)** for example JSON/XML/HTML outputs.
See **[docs/images/README.md](docs/images/README.md)** for screenshot capture guidelines.

## LCARS Providers & Demo
The LCARS reporter supports pluggable result providers so the final JSON and HTML can be driven by different sources (Palantír in-memory maps, Unreal's `AutomationTestFramework`, or future adapters).

Quick demo (no Unreal required):

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -File .\Scripts\Engage.ps1
```

To choose a different provider, add this to `Config/DefaultEngine.ini`:

```
[/Script/Nexus.Palantir]
LCARSSource=AutomationFramework
```

See `Docs/LCARS.md` for details on configuration, providers, and extending the reporter.
