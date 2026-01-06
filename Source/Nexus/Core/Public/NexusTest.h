#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Nexus/Palantir/Public/PalantirTrace.h"
#include "HAL/PlatformStackWalk.h"

// Forward declarations
class UNexusCore;
class AGameStateBase;
class APlayerController;
DECLARE_LOG_CATEGORY_EXTERN(LogNexus, Display, All);

/**
 * Performance metrics captured during test execution
 * Attached to FNexusTestContext for easy performance assertions
 */
struct NEXUS_API FTestPerformanceMetrics
{
    float AverageFPS = 0.0f;
    float PeakMemoryMb = 0.0f;
    int32 HitchCount = 0;
    bool bPassedPerformanceGates = true;
    
    bool IsValid() const
    {
        return AverageFPS > 0.0f || PeakMemoryMb > 0.0f;
    }
};

/**
 * Test execution result with diagnostic information
 * Captured for every test execution and used for reporting/analysis
 */
struct NEXUS_API FNexusTestResult
{
    FString TestName;
    bool bPassed = false;
    double DurationSeconds = 0.0;
    uint32 Attempts = 1;
    FString ErrorMessage;        // Error message if test failed
    TArray<FString> StackTrace;  // Stack trace on failure (diagnostic info)
    FDateTime Timestamp = FDateTime::Now();
    
    bool HasStackTrace() const
    {
        return StackTrace.Num() > 0;
    }
    
    FString GetStackTraceString() const
    {
        return FString::Join(StackTrace, TEXT("\n"));
    }
};

#if !defined(NEXUS_API)
    #define NEXUS_API
#endif

enum class ETestPriority : uint8
{
    Normal      = 0,
    Critical    = 1 << 0,   // Fail-fast on this
    Smoke       = 1 << 1,   // Run first
    OnlineOnly  = 1 << 2    // Requires network
};

// Enable bitwise operations on ETestPriority enum
ENUM_CLASS_FLAGS(ETestPriority);

inline bool NexusHasFlag(ETestPriority Flags, ETestPriority Check)
{
    return (static_cast<uint8>(Flags) & static_cast<uint8>(Check)) != 0;
}

enum class ETestTag : uint8
{
    None            = 0,
    Networking      = 1,     // Network/multiplayer tests
    Performance     = 2,     // Performance/benchmark tests
    Gameplay        = 4,     // Gameplay/mechanics tests
    Compliance      = 8,     // Compliance/regulation tests (COPPA/GDPR/DSA)
    Integration     = 16,    // Integration tests across modules
    Stress          = 32,    // Stress/load tests
    Editor          = 64,    // Editor-only tests
    Rendering       = 128    // Rendering/graphics tests
};

ENUM_CLASS_FLAGS(ETestTag);

inline bool NexusHasTag(ETestTag Tags, ETestTag Check)
{
    return (static_cast<uint8>(Tags) & static_cast<uint8>(Check)) != 0;
}

/**
 * FNexusTestContext - Provides test with access to game world, game state, and player controller
 * 
 * Only valid in NEXUS_TEST_GAMETHREAD tests. In parallel NEXUS_TEST, context will be nullptr.
 * Always check IsValid() before using context members.
 */
struct NEXUS_API FNexusTestContext
{
    UWorld* World = nullptr;
    AGameStateBase* GameState = nullptr;
    APlayerController* PlayerController = nullptr;
    
    // Tracked actors for automatic cleanup
    TArray<AActor*> SpawnedActors;
    
    // Performance metrics captured during test execution
    FTestPerformanceMetrics PerformanceMetrics;
    
    /**
     * Check if context is valid and safe to use
     * @return true if World is valid, false otherwise
     */
    bool IsValid() const
    {
        return World != nullptr && !World->bIsTearingDown;
    }
    
    /**
     * Spawn a test character with automatic cleanup tracking
     * @param CharClass Character class to spawn
     * @param Location Spawn location
     * @param Rotation Spawn rotation (default zero)
     * @return Spawned character, or nullptr if spawn failed
     * 
     * Note: const method - stored reference is tracked in mutable SpawnedActors array
     */
    ACharacter* SpawnTestCharacter(TSubclassOf<ACharacter> CharClass, FVector Location, FRotator Rotation = FRotator::ZeroRotator) const
    {
        if (!IsValid() || CharClass == nullptr)
        {
            return nullptr;
        }
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
        
        ACharacter* Character = World->SpawnActor<ACharacter>(CharClass, Location, Rotation, SpawnParams);
        if (Character)
        {
            // Explicitly cast ACharacter* to AActor* for storage in polymorphic array
            const_cast<FNexusTestContext*>(this)->SpawnedActors.Add(static_cast<AActor*>(Character));
        }
        return Character;
    }
    
    /**
     * Cleanup all spawned actors tracked by this context
     */
    void CleanupSpawnedActors()
    {
        for (AActor* Actor : SpawnedActors)
        {
            if (Actor && !Actor->IsActorBeingDestroyed())
            {
                Actor->Destroy();
            }
        }
        SpawnedActors.Empty();
    }
    
    /**
     * Check if performance metrics are available (ArgusLens ran during test)
     */
    bool HasPerformanceData() const
    {
        return PerformanceMetrics.IsValid();
    }
    
    /**
     * Assert that average FPS meets minimum threshold
     * @param MinFPS Minimum acceptable FPS
     * @return true if FPS >= MinFPS, false otherwise
     */
    bool AssertAverageFPS(float MinFPS) const
    {
        if (!HasPerformanceData())
        {
            UE_LOG(LogNexus, Warning, TEXT("No performance data available for FPS assertion"));
            return true;
        }
        
        if (PerformanceMetrics.AverageFPS < MinFPS)
        {
            UE_LOG(LogNexus, Error, TEXT("FPS assertion failed: %.1f < %.1f"), PerformanceMetrics.AverageFPS, MinFPS);
            return false;
        }
        return true;
    }
    
    /**
     * Assert that peak memory usage stays under limit
     * @param MaxMemoryMb Maximum acceptable memory in MB
     * @return true if memory <= MaxMemoryMb, false otherwise
     */
    bool AssertMaxMemory(float MaxMemoryMb) const
    {
        if (!HasPerformanceData())
        {
            UE_LOG(LogNexus, Warning, TEXT("No performance data available for memory assertion"));
            return true;
        }
        
        if (PerformanceMetrics.PeakMemoryMb > MaxMemoryMb)
        {
            UE_LOG(LogNexus, Error, TEXT("Memory assertion failed: %.0f > %.0f MB"), PerformanceMetrics.PeakMemoryMb, MaxMemoryMb);
            return false;
        }
        return true;
    }
    
    /**
     * Assert that hitch count stays under limit
     * @param MaxHitches Maximum acceptable number of hitches
     * @return true if hitches <= MaxHitches, false otherwise
     */
    bool AssertMaxHitches(int32 MaxHitches) const
    {
        if (!HasPerformanceData())
        {
            UE_LOG(LogNexus, Warning, TEXT("No performance data available for hitch assertion"));
            return true;
        }
        
        if (PerformanceMetrics.HitchCount > MaxHitches)
        {
            UE_LOG(LogNexus, Error, TEXT("Hitch assertion failed: %d > %d"), PerformanceMetrics.HitchCount, MaxHitches);
            return false;
        }
        return true;
    }
    
    ~FNexusTestContext()
    {
        // Auto-cleanup on destruction (RAII pattern)
        CleanupSpawnedActors();
    }
};

class NEXUS_API FNexusTest
{
public:
    FString TestName;
    ETestPriority Priority = ETestPriority::Normal;
    ETestTag Tags = ETestTag::None;  // Tags for filtering tests (Networking, Performance, etc.)
    TArray<FString> CustomTags;  // Custom string-based tags for project-specific categorization
    bool bRequiresGameThread = false;  // Flag for game-thread-only tests
    bool bSkip = false;  // Flag to skip test execution
    uint32 MaxRetries = 0;  // Number of times to retry on failure (default: 0 = no retries)
    double MaxDurationSeconds = 0.0;  // Maximum test duration in seconds (0 = unlimited)
    TFunction<bool(const FNexusTestContext&)> TestFunc;
    TFunction<bool(const FNexusTestContext&)> BeforeEach;  // Setup/fixture - called before each test attempt
    TFunction<void(const FNexusTestContext&)> AfterEach;   // Teardown/cleanup - called after each test attempt
    mutable FNexusTestResult LastResult;  // Result of last execution (mutable for const Execute())

    // Static list of all test instances - populated automatically at load time
    // when NEXUS_TEST() static objects are constructed
    static TArray<FNexusTest*> AllTests;
    static TArray<FNexusTestResult> AllResults;  // History of all test results for trend analysis

    FNexusTest(const FString& InName, ETestPriority InPriority, TFunction<bool(const FNexusTestContext&)> InFunc, bool bInRequiresGameThread = false, std::initializer_list<FString> Tags = {})
        : TestName(InName), Priority(InPriority), bRequiresGameThread(bInRequiresGameThread), TestFunc(MoveTemp(InFunc))
    {
        // Self-register into static list (no circular dependency!)
        AllTests.Add(this);
        
        // Initialize custom tags from parameter
        if (Tags.size() == 0)
        {
            // Default tag for backwards compatibility - all tests appear in reports
            AddCustomTag(TEXT("Untagged"));
        }
        else
        {
            for (const FString& Tag : Tags)
            {
                AddCustomTag(Tag);
            }
        }
    }

    /**
     * Add a custom tag to this test for project-specific categorization
     * @param CustomTag Tag string (e.g., "NexusTrials", "CinematicSequence", "MustPass")
     */
    void AddCustomTag(const FString& CustomTag)
    {
        if (!CustomTag.IsEmpty() && !CustomTags.Contains(CustomTag))
        {
            CustomTags.Add(CustomTag);
        }
    }

    /**
     * Check if test has a specific custom tag
     * @param CustomTag Tag string to search for
     * @return true if test has this custom tag, false otherwise
     */
    bool HasCustomTag(const FString& CustomTag) const
    {
        return CustomTags.Contains(CustomTag);
    }

    /**
     * Get all custom tags for this test
     * @return const reference to custom tags array
     */
    const TArray<FString>& GetCustomTags() const
    {
        return CustomTags;
    }

    bool Execute(const FNexusTestContext& Context = FNexusTestContext()) const
    {
        // Check if test should be skipped
        if (bSkip)
        {
            UE_LOG(LogNexus, Warning, TEXT("SKIPPED: %s"), *TestName);
            return true;  // Skipped tests count as passed
        }
        
        // RAII guard automatically creates and cleans up trace context
        FPalantirTraceGuard TraceGuard;
        
        const TCHAR* PriorityStr = NexusHasFlag(Priority, ETestPriority::Critical) ? TEXT("CRITICAL") : TEXT("NORMAL");
        UE_LOG_TRACE(LogNexus, Display, TEXT("RUNNING: %s [%s]"), *TestName, PriorityStr);
        PALANTIR_BREADCRUMB(TEXT("TestStart"), TestName);
        
        // Execute test with retry logic and timeout handling
        bool bResult = false;
        uint32 Attempt = 0;
        uint32 MaxAttempts = 1 + MaxRetries;  // Total attempts = 1 initial + retries
        
        while (Attempt < MaxAttempts)
        {
            ++Attempt;
            
            // Call setup fixture (BeforeEach) if provided
            bool bSetupSuccess = true;
            if (BeforeEach)
            {
                bSetupSuccess = BeforeEach(Context);
                if (!bSetupSuccess)
                {
                    UE_LOG(LogNexus, Error, TEXT("Setup fixture failed for %s"), *TestName);
                    bResult = false;
                }
            }
            
            // Run test only if setup succeeded
            if (bSetupSuccess)
            {
                double StartTime = FPlatformTime::Seconds();
                bResult = TestFunc(Context);
                double Duration = FPlatformTime::Seconds() - StartTime;
                
                // Check if test exceeded timeout
                if (MaxDurationSeconds > 0.0 && Duration > MaxDurationSeconds)
                {
                    UE_LOG(LogNexus, Error, TEXT("TIMEOUT: %s exceeded max duration: %.2fs > %.2fs"), 
                        *TestName, Duration, MaxDurationSeconds);
                    PALANTIR_BREADCRUMB(TEXT("Timeout"), 
                        FString::Printf(TEXT("Duration: %.2fs, Limit: %.2fs"), Duration, MaxDurationSeconds));
                    bResult = false;  // Timeout = test failure
                }
            }
            
            // Call teardown fixture (AfterEach) if provided - always called regardless of test result
            if (AfterEach)
            {
                AfterEach(Context);
            }
            
            if (bResult)
            {
                // Test passed
                if (Attempt > 1)
                {
                    UE_LOG(LogNexus, Display, TEXT("PASSED after %d attempts: %s"), Attempt, *TestName);
                }
                break;
            }
            else if (Attempt < MaxAttempts)
            {
                // Test failed but we have retries remaining - use exponential backoff
                double WaitTime = FMath::Pow(2.0, Attempt - 1);  // 1s, 2s, 4s, 8s, etc.
                UE_LOG(LogNexus, Warning, TEXT("RETRY: %s failed attempt %d/%d, waiting %.1fs before retry"), 
                    *TestName, Attempt, MaxAttempts, WaitTime);
                FPlatformProcess::Sleep(WaitTime);
            }
        }
        
        PALANTIR_BREADCRUMB(TEXT("TestEnd"), 
            FString::Printf(TEXT("Attempts: %d, Status: %s"), 
                Attempt, bResult ? TEXT("PASS") : TEXT("FAIL")));
        
        UE_LOG_TRACE(LogNexus, Display, TEXT("COMPLETED: %s [%s] (attempt %d/%d)"), 
            *TestName, bResult ? TEXT("PASS") : TEXT("FAIL"), Attempt, MaxAttempts);
        
        // Capture result for history tracking and failure diagnostics
        LastResult.TestName = TestName;
        LastResult.bPassed = bResult;
        LastResult.Attempts = Attempt;
        LastResult.Timestamp = FDateTime::Now();
        
        // Capture stack trace on failure for diagnostics
        if (!bResult)
        {
            LastResult.ErrorMessage = FString::Printf(TEXT("Test failed after %d attempt(s)"), Attempt);
            
            // Capture stack trace (if available on this platform)
            // UE 5.7 API: StackWalkAndDump expects ANSICHAR buffer
            const int32 MaxFrames = 32;
            const int32 BufferSize = MaxFrames * 256;  // Each frame ~256 chars
            ANSICHAR* StackBuffer = static_cast<ANSICHAR*>(FMemory::Malloc(BufferSize));
            
            FPlatformStackWalk::StackWalkAndDump(StackBuffer, BufferSize, nullptr);
            
            // Convert ANSI buffer to FString and parse into lines
            FString CallStack(StackBuffer);
            FMemory::Free(StackBuffer);
            
            // Parse call stack into lines (UE 5.7 API: array passed by reference)
            TArray<FString> Lines;
            CallStack.ParseIntoArray(Lines, TEXT("\n"), true);
            LastResult.StackTrace = Lines;
        }
        
        // Store in global history for trend analysis
        AllResults.Add(LastResult);
        
        return bResult;
    }
    
    /**
     * Check if this test has all specified tags
     * @param RequiredTags Tags to check for
     * @return true if test has all required tags, false otherwise
     */
    bool HasTags(ETestTag RequiredTags) const
    {
        if (RequiredTags == ETestTag::None)
        {
            return true;  // No filtering
        }
        
        // Check if test has all required tags
        return NexusHasTag(Tags, RequiredTags);
    }
};

// Use this instead of IMPLEMENT_SIMPLE_AUTOMATION_TEST
// Tests self-register via FNexusTest constructor - no circular dependency!

// Internal macro implementation - handles both parallel-safe and game-thread-only tests
#define NEXUS_TEST_INTERNAL(TestClassName, PrettyName, PriorityFlags, bGameThreadOnly, ...) \
class TestClassName : public FNexusTest \
{ \
public: \
    TestClassName() : FNexusTest(PrettyName, PriorityFlags, [this](const FNexusTestContext& Context) -> bool { return RunTest(Context); }, bGameThreadOnly, __VA_ARGS__) {} \
    bool RunTest(const FNexusTestContext& Context); \
}; \
static TestClassName Global_##TestClassName; \
bool TestClassName::RunTest(const FNexusTestContext& Context)

// ============================================================================
// Test Macros Without Tags (Backwards Compatible)
// ============================================================================

// Standard test - parallel-safe, no tags (gets "Untagged" automatically)
#define NEXUS_TEST(TestClassName, PrettyName, PriorityFlags) \
    NEXUS_TEST_INTERNAL(TestClassName, PrettyName, PriorityFlags, false, {})

// Game-thread-only test - no tags (gets "Untagged" automatically)
#define NEXUS_TEST_GAMETHREAD(TestClassName, PrettyName, PriorityFlags) \
    NEXUS_TEST_INTERNAL(TestClassName, PrettyName, PriorityFlags, true, {})

// Performance test - no tags (gets "Untagged" automatically)
// Usage: NEXUS_PERF_TEST(FMyPerfTest, "Perf.CPU.Rendering", ETestPriority::Normal, 60.0f) { ... }
#define NEXUS_PERF_TEST(TestClassName, PrettyName, PriorityFlags, DurationSeconds) \
class TestClassName : public FNexusTest \
{ \
public: \
    TestClassName() : FNexusTest(PrettyName, PriorityFlags, [this](const FNexusTestContext& Context) -> bool { return RunPerformanceTest(Context); }, true, {}) {} \
    bool RunPerformanceTest(const FNexusTestContext& Context); \
}; \
static TestClassName Global_##TestClassName; \
bool TestClassName::RunPerformanceTest(const FNexusTestContext& Context)

// ============================================================================
// Test Macros With Tags (New)
// ============================================================================

// Standard test with custom tags
// Usage: NEXUS_TEST_TAGGED(FMyTest, "Module.Feature", ETestPriority::Normal, {"Networking", "P1"})
#define NEXUS_TEST_TAGGED(TestClassName, PrettyName, PriorityFlags, ...) \
    NEXUS_TEST_INTERNAL(TestClassName, PrettyName, PriorityFlags, false, __VA_ARGS__)

// Game-thread-only test with custom tags
// Usage: NEXUS_TEST_GAMETHREAD_TAGGED(FMyTest, "Module.Feature", ETestPriority::Normal, {"Gameplay", "Critical"})
#define NEXUS_TEST_GAMETHREAD_TAGGED(TestClassName, PrettyName, PriorityFlags, ...) \
    NEXUS_TEST_INTERNAL(TestClassName, PrettyName, PriorityFlags, true, __VA_ARGS__)

// Performance test with custom tags
// Usage: NEXUS_PERF_TEST_TAGGED(FMyPerfTest, "Perf.CPU.Rendering", ETestPriority::Normal, 60.0f, {"Performance", "P1"})
#define NEXUS_PERF_TEST_TAGGED(TestClassName, PrettyName, PriorityFlags, DurationSeconds, ...) \
class TestClassName : public FNexusTest \
{ \
public: \
    TestClassName() : FNexusTest(PrettyName, PriorityFlags, [this](const FNexusTestContext& Context) -> bool { return RunPerformanceTest(Context); }, true, __VA_ARGS__) {} \
    bool RunPerformanceTest(const FNexusTestContext& Context); \
}; \
static TestClassName Global_##TestClassName; \
bool TestClassName::RunPerformanceTest(const FNexusTestContext& Context)

// Performance assertion helpers - use in tests to validate metrics
#define ASSERT_AVERAGE_FPS(Context, MinFPS) \
    if (!(Context).AssertAverageFPS(MinFPS)) { return false; }

#define ASSERT_MAX_MEMORY(Context, MaxMemoryMb) \
    if (!(Context).AssertMaxMemory(MaxMemoryMb)) { return false; }

#define ASSERT_MAX_HITCHES(Context, MaxHitches) \
    if (!(Context).AssertMaxHitches(MaxHitches)) { return false; }

// Check if performance data is available (ArgusLens ran)
#define HAS_PERF_DATA(Context) ((Context).HasPerformanceData())
