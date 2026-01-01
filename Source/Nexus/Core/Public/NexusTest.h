#pragma once
#include "CoreMinimal.h"
#include "Nexus/Palantir/Public/PalantirTrace.h"

// Forward declarations
class UNexusCore;
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

class NEXUS_API FNexusTest
{
public:
    FString TestName;
    ETestPriority Priority = ETestPriority::Normal;
    TFunction<bool()> TestFunc;

    // Static list of all test instances - populated automatically at load time
    // when NEXUS_TEST() static objects are constructed
    static NEXUS_API TArray<FNexusTest*> AllTests;

    FNexusTest(const FString& InName, ETestPriority InPriority, TFunction<bool()> InFunc)
        : TestName(InName), Priority(InPriority), TestFunc(MoveTemp(InFunc))
    {
        // Self-register into static list (no circular dependency!)
        AllTests.Add(this);
    }

    bool Execute() const
    {
        // RAII guard automatically creates and cleans up trace context
        FPalantirTraceGuard TraceGuard;
        
        const TCHAR* PriorityStr = NexusHasFlag(Priority, ETestPriority::Critical) ? TEXT("CRITICAL") : TEXT("NORMAL");
        UE_LOG_TRACE(LogNexus, Display, TEXT("RUNNING: %s [%s]"), *TestName, PriorityStr);
        PALANTIR_BREADCRUMB(TEXT("TestStart"), TestName);
        
        double StartTime = FPlatformTime::Seconds();
        bool bResult = TestFunc();
        double Duration = FPlatformTime::Seconds() - StartTime;
        
        PALANTIR_BREADCRUMB(TEXT("TestEnd"), 
            FString::Printf(TEXT("Duration: %.3fs, Status: %s"), 
                Duration, bResult ? TEXT("PASS") : TEXT("FAIL")));
        
        UE_LOG_TRACE(LogNexus, Display, TEXT("COMPLETED: %s [%s] (%.3fs)"), 
            *TestName, bResult ? TEXT("PASS") : TEXT("FAIL"), Duration);
        
        return bResult;
    }
};

// Use this instead of IMPLEMENT_SIMPLE_AUTOMATION_TEST
// Tests self-register via FNexusTest constructor - no circular dependency!
#define NEXUS_TEST(TestClassName, PrettyName, PriorityFlags) \
class TestClassName : public FNexusTest \
{ \
public: \
    TestClassName() : FNexusTest(PrettyName, PriorityFlags, [this]() -> bool { return RunTest(); }) {} \
    bool RunTest(); \
}; \
static TestClassName Global_##TestClassName; \
bool TestClassName::RunTest()