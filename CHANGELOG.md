# Changelog

All notable changes to the NexusQA framework are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased] - Current Development

### Added

#### Skip Test Support (New Feature)
- **NEXUS_SKIP_TEST(reason)** macro for conditional test skipping
  - Skip tests based on runtime conditions (network unavailable, feature disabled, platform-specific)
  - Skipped tests tracked separately from pass/fail in reports and dashboard
  - Skip reasons logged for debugging and artifact generation
  - Example: `if (!IsNetworkAvailable()) { NEXUS_SKIP_TEST("Network unavailable"); }`

#### Enhanced Slate Dashboard
- New **Test Status section** showing real-time counters:
  - ✓ Passed tests (green)
  - ⚠️ Failed tests (red)
  - ⏭️ Skipped tests (gold)
  - 📊 Total tests vs discovered
- Improved window properties:
  - 700x600 configurable window with user sizing
  - Keyboard focus and window bring-to-front
  - Professional appearance with emoji icons
- Event categorization with skip indicator:
  - ⏭️ icon for skipped events in gold color
  - Proper categorization alongside blocked/failed/success events

#### Auto-PIE (Play In Editor) Support
- Automatic world initialization for game-thread tests
- RequestPlaySession with intelligent waiting (up to 5 seconds)
- Graceful fallback if Auto-PIE not configured
- Documentation: [NEXUS_GUIDE.md - Running Game-Thread Tests with Auto-PIE](Docs/NEXUS_GUIDE.md#running-game-thread-tests-with-auto-pie)

#### Network-Aware Testing
- IsNetworkAvailable() helper for DNS-based connectivity detection
- All 8 HTTP tests now skip gracefully when network unavailable
- No timeout failures in offline environments
- Proper network state logging

### Improved

#### LCARS Report Generation
- Fixed test categorization (all 34 tests now appear in category distribution)
- Improved data sourcing: Reports now read from stored test metadata during execution
- Skip count display in reports alongside passed/failed counts
- Integrity percentage calculated from executed tests (excluding skips)
- Better handling of directory vs file paths for export

#### Test Execution
- Fixed double-counting of test results (removed duplicate NotifyTestFinished call)
- Improved test result tracking with proper metadata storage
- Performance metrics now read from correctly populated data sources
- Better handling of skipped tests in execution paths

#### Documentation
- Comprehensive skip test patterns with examples in [NEXUS_GUIDE.md](Docs/NEXUS_GUIDE.md#skipping-tests-conditionally)
- Updated [CONTRIBUTING.md](CONTRIBUTING.md) with skip examples and game-thread test patterns
- Updated [OBSERVER_NETWORK_GUIDE.md](Docs/OBSERVER_NETWORK_GUIDE.md) with Slate dashboard features
- Auto-PIE configuration guide in NEXUS_GUIDE.md

### Fixed

#### Macro System
- Fixed preprocessor parameter counting with initializer lists using `__VA_ARGS__`
- Applied to all test macros: NEXUS_TEST_INTERNAL, NEXUS_TEST_TAGGED, NEXUS_TEST_GAMETHREAD_TAGGED, NEXUS_PERF_TEST_TAGGED
- Backward compatible with existing test code

#### API Compatibility
- Replaced deprecated FindObject boolean parameter with EFindObjectFlags::ExactClass
- Forward-compatible with UE5.7+ deprecation warnings

#### Test Framework
- OnTestSkipped() function added to PalantirObserver for skip event handling
- NotifyTestSkipped() in NexusCore for skip count tracking
- Proper skip detection in both parallel and sequential execution paths

---

## [v1.0.0] - Current Release

### Features
- ✅ **Parallel Test Execution** - Run 100s of tests simultaneously using thread pool
- ✅ **Fail-Fast Execution** - Stop on critical test failure to save time
- ✅ **Result Aggregation** - Track pass/fail/skip, duration, error messages, stack traces
- ✅ **Console Commands** - Run tests via `Nexus.RunTests` command
- ✅ **LCARS Integration** - Export results to enhanced HTML/JSON/XML reports
- ✅ **Performance Monitoring** - Real-time FPS, memory, hitch tracking
- ✅ **Network Chaos** - Lag injection, packet loss, disconnect simulation
- ✅ **Compliance Testing** - COPPA/GDPR/DSA validation
- ✅ **API Testing** - REST/GraphQL fluent assertions
- ✅ **Live Dashboard** - Slate in-editor monitoring with real-time counters

### Module Overview
| Module | Status | Key Features |
|--------|--------|--------------|
| **Nexus** | ✅ Stable | Test orchestration, parallel execution, fail-fast |
| **Palantír** | ✅ Stable | Distributed tracing, API testing, test metadata |
| **LCARSBridge** | ✅ Stable | HTML/JSON/XML reports with enhanced categorization |
| **FringeNetwork** | ✅ Stable | Network chaos engineering, event logging |
| **StargateStress** | ✅ Stable | Load testing, bot simulation |
| **ArgusLens** | ✅ Stable | Performance monitoring, metrics collection |
| **Protego** | ✅ Stable | Compliance validation, accessibility checks |
| **Observer** | ✅ Stable | Live dashboard, safety event tracking |

### Known Limitations
- ImGui dashboard requires `WITH_IMGUI=1` compilation flag for live rendering
- Network detection uses DNS lookup (requires some network interface)
- Auto-PIE requires DefaultGame.ini TestMapPath configuration for full support
- Performance metrics collection requires ArgusLens module initialization

---

## How to Contribute

We welcome contributions! Please follow [CONTRIBUTING.md](CONTRIBUTING.md) guidelines:
- Include tests for new features (aim for >80% coverage)
- Use NEXUS_TEST macros with appropriate tags
- Consider skip patterns for environment-specific functionality
- Update documentation for user-facing changes
- Run full test suite before submitting PR

---

## Support

For questions or issues:
1. Check relevant documentation in `Docs/` folder
2. Review example tests in `Source/*/Private/Tests/`
3. Consult [ARCHITECTURE.md](ARCHITECTURE.md) for design patterns
4. Open an issue with reproduction steps and logs
