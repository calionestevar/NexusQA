# LCARS Reporter & Providers Guide

## What is LCARS?

**LCARS** (Library Computer Access and Retrieval System) is the Nerdy QA Toolkit's test reporting system, named after Star Trek's fictional starship computer interface. It provides:

1. **Multiple Output Formats:**
   - JSON (machine-readable results)
   - XML (JUnit format for CI/CD)
   - HTML (Starfleet-themed visual dashboard)

2. **Pluggable Providers:**
   - Palantír (in-memory test results)
   - AutomationTestFramework (UE5 native automation)
   - Custom implementations (extensible)

3. **Rich Visualizations:**
   - Test pass/fail summary with statistics
   - API testing metrics (request counts, response times, status codes)
   - Performance tracking (FPS, memory, hitches)
   - Trace ID correlation and artifact links

---

## LCARS Providers Overview

LCARS Providers are pluggable implementations of the `ILCARSResultsProvider` interface that allow the LCARS reporting system to source test results from different backends.

## Architecture

```
ILCARSResultsProvider (abstract interface)
    ↓
    ├── FPalantirLCARSProvider (reads from PalantirOracle)
    ├── FAutomationTestLCARSProvider (reads from UE5 AutomationTestFramework)
    └── Custom Implementations
         ↓
    FLCARSProviderFactory (creates and manages instances)
```

## Built-in Providers

### 1. FPalantirLCARSProvider

**Purpose:** Reads test results from Palantír's in-memory test execution records.

**When to use:** In Nexus-orchestrated tests that use PalantirOracle.

**Usage:**
```cpp
#include "PalantirLCARSProvider.h"
#include "LCARSProviderFactory.h"

FPalantirOracle* Oracle = GetPalantirOracle();
TUniquePtr<ILCARSResultsProvider> Provider = 
    FLCARSProviderFactory::CreateProvider(ELCARSProviderType::Palantir, Oracle);

FLCARSResults Results = Provider->GetResults();
```

**Data Sources:**
- Test pass/fail status from `FPalantirTestResult::bPassed`
- Test duration from `FPalantirTestResult::Duration`
- Artifacts: screenshots, trace files, logs

---

### 2. FAutomationTestLCARSProvider

**Purpose:** Reads test results from Unreal Engine's native AutomationTestFramework.

**When to use:** For UE5 built-in automation tests or third-party automation frameworks.

**Status:** Ready for integration with FAutomationTestFramework when native automation tests are added.

**Usage:**
```cpp
#include "AutomationTestLCARSProvider.h"
#include "LCARSProviderFactory.h"

TUniquePtr<ILCARSResultsProvider> Provider = 
    FLCARSProviderFactory::CreateProvider(ELCARSProviderType::AutomationTest);

FLCARSResults Results = Provider->GetResults();
```

**Future Integration:**
When connected to the automation framework, this provider will extract:
- Test results from FAutomationTestFramework
- Timing information
- Screenshots and artifacts
- Error messages and logs

---

## Creating Custom Providers

### Step 1: Implement ILCARSResultsProvider

```cpp
#pragma once

#include "CoreMinimal.h"
#include "LCARSProvider.h"

class FMyCustomLCARSProvider : public ILCARSResultsProvider
{
public:
    virtual ~FMyCustomLCARSProvider() = default;
    
    virtual FLCARSResults GetResults() override
    {
        FLCARSResults Results;
        
        // Populate from your data source
        Results.Results.Add(TEXT("MyTest_001"), true);
        Results.Durations.Add(TEXT("MyTest_001"), 1.234);
        
        // Add artifacts
        TArray<FString> Artifacts;
        Artifacts.Add(TEXT("path/to/screenshot.png"));
        Results.Artifacts.Add(TEXT("MyTest_001"), Artifacts);
        
        return Results;
    }
};
```

### Step 2: Register with Factory

```cpp
#include "LCARSProviderFactory.h"

// In your initialization code:
FLCARSProviderFactory::RegisterCustomProvider(
    TEXT("MyCustomProvider"),
    []() { return MakeUnique<FMyCustomLCARSProvider>(); }
);

// Later, create the provider:
TUniquePtr<ILCARSResultsProvider> Provider = 
    FLCARSProviderFactory::CreateCustomProvider(TEXT("MyCustomProvider"));
```

---

## Data Structures

### FLCARSResults

```cpp
struct FLCARSResults
{
    // Test name -> pass/fail status
    TMap<FString, bool> Results;
    
    // Test name -> duration in seconds
    TMap<FString, double> Durations;
    
    // Test name -> array of artifact file paths
    TMap<FString, TArray<FString>> Artifacts;
};
```

### Artifact Types

Common artifact types supported:
- **Screenshots:** `.png`, `.jpg` (from PalantirCapture)
- **Traces:** `.json` (trace timeline data)
- **Logs:** `.txt`, `.log` (test execution logs)
- **Reports:** `.xml`, `.csv` (result reports)

---

## Integration Examples

### With HTML Report Generation

```cpp
#include "LCARSHTMLGenerator.h"
#include "LCARSProviderFactory.h"

FPalantirOracle* Oracle = GetOracle();
TUniquePtr<ILCARSResultsProvider> Provider = 
    FLCARSProviderFactory::CreateProvider(ELCARSProviderType::Palantir, Oracle);

FLCARSResults Results = Provider->GetResults();

// Convert to report data structure
FLCARSHTMLGenerator::FReportData ReportData;
// ... populate from Results
// Generate HTML
FString HTML = FLCARSHTMLGenerator::GenerateHTML(ReportData);
```

### With CI/CD Pipeline

```cpp
// In commandlet or CI integration
class UGenerateLCARSReportCommandlet : public UCommandlet
{
    virtual int32 Main(const FString& Params) override
    {
        // Determine which provider to use based on environment
        ELCARSProviderType ProviderType = DetermineProvider();
        TUniquePtr<ILCARSResultsProvider> Provider = 
            FLCARSProviderFactory::CreateProvider(ProviderType);
        
        FLCARSResults Results = Provider->GetResults();
        // Convert to HTML and save
        
        return 0;
    }
};
```

---

## Best Practices

### 1. Provider Selection
- Use **Palantir** for Nexus-orchestrated tests
- Use **AutomationTest** for UE5 native tests
- Create **Custom** providers for external test systems (Jenkins, CI runners, etc.)

### 2. Error Handling
Always check for null providers:
```cpp
TUniquePtr<ILCARSResultsProvider> Provider = FLCARSProviderFactory::CreateProvider(Type, Context);
if (!Provider)
{
    UE_LOG(LogTemp, Error, TEXT("Failed to create provider"));
    return;
}
```

### 3. Caching
Some providers (like AutomationTest) cache results. Call `GetResults()` after test execution completes.

### 4. Thread Safety
Providers should be safe to call from the game thread. If implementing custom providers that access external systems, add appropriate synchronization.

---

## Extending the Provider System

### Adding a New Provider Type

1. Create header in `Public/`:
   ```
   Source/Nexus/Palantir/Public/YourCustomProvider.h
   ```

2. Create implementation in `Private/`:
   ```
   Source/Nexus/Palantir/Private/YourCustomProvider.cpp
   ```

3. Update `FLCARSProviderFactory::CreateProvider()` to handle the new type

4. Add factory support if creating instances by name

---

## FAQ

**Q: Can I use multiple providers together?**
A: Not directly, but you can create a composite provider that aggregates results from multiple sources.

**Q: Do providers persist state between calls?**
A: It depends on the implementation. Palantir keeps in-memory results for the session. Custom providers should document their behavior.

**Q: How do I add custom metadata to results?**
A: Extend `FLCARSResults` struct or create a subclass of the provider interface with additional methods.

---

## Related Files

- `Source/Nexus/Palantir/Public/LCARSProvider.h` - Base interface
- `Source/Nexus/Palantir/Public/LCARSProviderFactory.h` - Factory class
- `Source/Nexus/Palantir/Public/PalantirLCARSProvider.h` - Palantir implementation
- `Source/Nexus/Palantir/Public/AutomationTestLCARSProvider.h` - Automation implementation
- `Source/Nexus/Palantir/Public/LCARSHTMLGenerator.h` - Report generation
