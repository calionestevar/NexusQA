# NexusQA Architecture

## Current Design (Monolithic but Cohesive)

The framework is currently organized as a single **Nexus** module with internal sub-components:

```
Nexus/
├── Core/
│   ├── NexusCore.h/cpp       (Test orchestration & discovery)
│   └── NexusTest.h           (Test macro & registration)
├── Palantir/
│   ├── PalantirTrace.h/cpp   (Distributed tracing)
│   ├── PalantirRequest.h/cpp (HTTP testing API)
│   ├── PalantirOracle.h/cpp  (Reporting aggregation)
│   └── PalantirVision.h      (Assertions & validation)
└── LCARSBridge/
    ├── LCARSHTMLGenerator.h  (Report generation)
    └── LCARSProvider.h       (Extensible result provider)
```

### Why This Structure?

1. **Single Module = Simple Deployment** - Easier for game projects to integrate
2. **Clear Concerns** - Each sub-component has a distinct responsibility
3. **Loose Coupling** - Palantir and LCARS can be extended independently
4. **Low Overhead** - No cross-module dependency complexity

## Design Patterns Used

### 1. **RAII for Context Management**
```cpp
FPalantirTraceGuard Guard;  // Auto-generates trace ID
// ... test code ...
}  // Guard destructor clears context
```
Ensures trace context cleanup without manual bookkeeping.

### 2. **Fluent Builder API (Palantir Requests)**
```cpp
FPalantirRequest::Get("https://api.example.com/users")
    .WithHeader("Authorization", "Bearer <token>")
    .ExpectStatus(200)
    .ExpectJSON("$.name", "John")
    .ExecuteBlocking();
```
Makes API tests readable and composable.

### 3. **Static Facade with Thread-Local State**
`FPalantirTrace` provides a simple static API while maintaining thread safety through `thread_local` storage.

### 4. **Pluggable Reporting (ILCARSResultsProvider)**
Multiple report generators can implement `ILCARSResultsProvider`:
- `FAutomationTestLCARSProvider` - Wraps UE5 automation results
- `FPalantirLCARSProvider` - Wraps Nexus test results
- Custom providers can be added without modifying core

## Future Refactoring Path (Post-MVP)

### Phase 1: Extract PalantirCore Module (v2.0)

**Goal:** Make distributed tracing reusable across non-test contexts.

```
PalantirCore/       ← New module
├── FPalantirTrace
├── FPalantirTraceGuard
├── UPalantirConfig
└── Plugins/
    └── DataDog/    ← Export integration

Nexus/              ← Simplified
├── Core/           (depends on PalantirCore)
├── Orchestration/
└── Reporting/
```

**Benefits:**
- Reuse tracing in gameplay code, replays, matchmaking
- DataDog/ElasticAPM integration independent of testing
- Smaller, focused module

### Phase 2: Extract LCARS Reporting (v2.5)

**Goal:** Support multiple reporting backends without core coupling.

```
Nexus/
├── Core/           (orchestration only)
└── Interfaces/
    └── ITestObserver

LCARS/              ← New module
├── FLCARSHTMLGenerator
├── Providers/
│   ├── AutomationTest
│   └── Nexus
└── Exporters/
    ├── JSON
    ├── XML
    └── S3Uploader
```

**Benefits:**
- Generate reports in CI/CD without test runner
- Multiple output formats
- Cloud upload (S3, Google Cloud)

### Phase 3: Interface-Driven Design (v3.0)

**Current:**
```cpp
// Hard dependency
class FNexusTest {
    FPalantirTraceGuard Guard;
    FPalantirObserver::OnTestStarted(Name);
    // ...
};
```

**Future:**
```cpp
// Dependency injection
class FNexusTest {
    ITestObserver* Observer;  // Interface
    ITraceProvider* Tracer;   // Interface
    // Allows mocking, swapping implementations
};
```

## Current Trade-offs

| Aspect | Current | Benefit | Cost |
|--------|---------|---------|------|
| **Cohesion** | High (single module) | Easy deployment | Harder to reuse components |
| **Coupling** | Moderate (static facades) | Simple to use | Hard to mock/test |
| **Extensibility** | Via interfaces (LCARS) | Some flexibility | Not fully pluggable |
| **Module Size** | ~5000 LOC | Maintainable | Could split into 3 modules |

## Adding New Features

### Scenario: Add DataDog Integration

**Current (v1.0):**
1. Modify `FPalantirTrace` to export DataDog format
2. Add `#if ENABLE_DATADOG` guards
3. Recompile entire Nexus module

**Future (v2.0+):**
1. Create `PalantirDataDogPlugin` module
2. Implements `ITraceExporter` interface
3. No core changes needed

## Backward Compatibility

All planned refactorings maintain **API compatibility**:
- `NEXUS_TEST` macro unchanged
- `FPalantirRequest` fluent API unchanged
- Module dependency order: PalantirCore → Nexus → LCARS

## References

**Architectural Patterns:**
- SOLID Principles (S: Single Responsibility, D: Dependency Inversion)
- Facade Pattern (static API over complex internal state)
- RAII (C++ resource management)
- Builder Pattern (fluent API design)

**Testing Frameworks Studied:**
- Unreal Automation (monolithic)
- Google Test (pluggable with interfaces)
- Catch2 (composable reporters)
- Cypress (fluent API for browser testing)

**APM/Tracing Systems:**
- DataDog APM (User-Agent trace injection)
- Jaeger/OpenTelemetry (trace context propagation)
- Garmin's internal system (correlation IDs)
- Cerner's custom identifiers

## When to Refactor

Consider extracting PalantirCore when:
- ✅ Non-test gameplay code needs distributed tracing
- ✅ Multiple projects want to reuse trace infrastructure
- ✅ DataDog/Sentry integration becomes project requirement

Not needed if:
- ❌ Framework only used for test execution
- ❌ Single game project
- ❌ No multi-system integration needed
