#pragma once
#include "CoreMinimal.h"
#include "Nexus/Palantir/Public/PalantirTrace.h"

// Forward declarations
class UNexusCore;
class AGameStateBase;
class APlayerController;
DECLARE_LOG_CATEGORY_EXTERN(LogNexus, Display, All);

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
     */
    ACharacter* SpawnTestCharacter(TSubclassOf<ACharacter> CharClass, FVector Location, FRotator Rotation = FRotator::ZeroRotator)
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
            SpawnedActors.Add(Character);
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
    bool bRequiresGameThread = false;  // Flag for game-thread-only tests
    bool bSkip = false;  // Flag to skip test execution
    uint32 MaxRetries = 0;  // Number of times to retry on failure (default: 0 = no retries)
    TFunction<bool(const FNexusTestContext&)> TestFunc;

    // Static list of all test instances - populated automatically at load time
    // when NEXUS_TEST() static objects are constructed
    static TArray<FNexusTest*> AllTests;

    FNexusTest(const FString& InName, ETestPriority InPriority, TFunction<bool(const FNexusTestContext&)> InFunc, bool bInRequiresGameThread = false)
        : TestName(InName), Priority(InPriority), bRequiresGameThread(bInRequiresGameThread), TestFunc(MoveTemp(InFunc))
    {
        // Self-register into static list (no circular dependency!)
        AllTests.Add(this);
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
        
        // Execute test with retry logic
        bool bResult = false;
        uint32 Attempt = 0;
        uint32 MaxAttempts = 1 + MaxRetries;  // Total attempts = 1 initial + retries
        
        while (Attempt < MaxAttempts)
        {
            ++Attempt;
            
            double StartTime = FPlatformTime::Seconds();
            bResult = TestFunc(Context);
            double Duration = FPlatformTime::Seconds() - StartTime;
            
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
        
        return bResult;
    }
};

// Use this instead of IMPLEMENT_SIMPLE_AUTOMATION_TEST
// Tests self-register via FNexusTest constructor - no circular dependency!

// Internal macro implementation - handles both parallel-safe and game-thread-only tests
#define NEXUS_TEST_INTERNAL(TestClassName, PrettyName, PriorityFlags, bGameThreadOnly) \
class TestClassName : public FNexusTest \
{ \
public: \
    TestClassName() : FNexusTest(PrettyName, PriorityFlags, [this](const FNexusTestContext& Context) -> bool { return RunTest(Context); }, bGameThreadOnly) {} \
    bool RunTest(const FNexusTestContext& Context); \
}; \
static TestClassName Global_##TestClassName; \
bool TestClassName::RunTest(const FNexusTestContext& Context)

// Original macro - defaults to false (parallel-safe)
#define NEXUS_TEST(TestClassName, PrettyName, PriorityFlags) \
    NEXUS_TEST_INTERNAL(TestClassName, PrettyName, PriorityFlags, false)

// New macro for game-thread-only tests
#define NEXUS_TEST_GAMETHREAD(TestClassName, PrettyName, PriorityFlags) \
    NEXUS_TEST_INTERNAL(TestClassName, PrettyName, PriorityFlags, true)