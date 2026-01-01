#include "NexusCore.h"

NEXUS_TEST(FNexusIsAliveTest, "Nexus.Smoke.DummyTest_ProvesFrameworkWorks", ETestPriority::Smoke)
{
    UE_LOG(LogTemp, Warning, TEXT("NEXUS IS ALIVE — THE REVOLUTION HAS BEGUN"));
    bool bResult = (6 * 7 == 42);
    if (!bResult)
    {
        UE_LOG(LogTemp, Error, TEXT("42 check failed"));
    }
    return bResult;
}

NEXUS_TEST(FNexusCanFailTest, "Nexus.Smoke.DummyTest_CanDetectFailure", ETestPriority::Smoke)
{
    // This test intentionally fails to verify fail detection works
    UE_LOG(LogTemp, Error, TEXT("Intentional failure to verify fail detection"));
    return false;  // Intentional failure
}

NEXUS_TEST_GAMETHREAD(FNexusContextTest, "Nexus.GameThread.ContextAccess", ETestPriority::Normal)
{
    // Demonstrate test context usage
    // This test shows how to access world, game state, and player controller
    
    if (!Context.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Test context not available (no world running) - skipping context tests"));
        return true;  // Pass if no world - this is expected in some test scenarios
    }
    
    UE_LOG(LogTemp, Display, TEXT("Test context is valid!"));
    UE_LOG(LogTemp, Display, TEXT("  World: %s"), Context.World ? TEXT("Valid") : TEXT("nullptr"));
    UE_LOG(LogTemp, Display, TEXT("  GameState: %s"), Context.GameState ? TEXT("Valid") : TEXT("nullptr"));
    UE_LOG(LogTemp, Display, TEXT("  PlayerController: %s"), Context.PlayerController ? TEXT("Valid") : TEXT("nullptr"));
    
    return true;
}

// Example: Skip tests based on conditions (uncomment to use)
// You can set bSkip dynamically from commandline or config
NEXUS_TEST(FNexusSkipExampleTest, "Nexus.Skip.ExampleSkipTest", ETestPriority::Normal)
{
    // Example: This test would be skipped if certain conditions are met
    // Uncomment the line below to see skip in action during testing
    // bSkip = true;
    
    UE_LOG(LogTemp, Display, TEXT("This test would be skipped if bSkip were set to true"));
    return true;
}

// Example: Retry tests that might be flaky
// This test demonstrates retry logic - it will retry on failure with exponential backoff
class FNexusRetryExampleTest : public FNexusTest
{
public:
    FNexusRetryExampleTest() : FNexusTest(
        TEXT("Nexus.Retry.ExampleFlakeyTest"),
        ETestPriority::Normal,
        [this](const FNexusTestContext& Context) -> bool { return RunTest(Context); },
        false)
    {
        // Uncomment the line below to enable retry (will retry up to 3 times on failure)
        // MaxRetries = 3;
    }
    
    bool RunTest(const FNexusTestContext& Context)
    {
        // Example flaky test that might fail randomly
        // In reality, you'd set MaxRetries on actual flaky tests
        UE_LOG(LogTemp, Display, TEXT("Retry example: if this test fails, it would be retried (MaxRetries=%d)"), MaxRetries);
        return true;  // Always passes in this example
    }
};

static FNexusRetryExampleTest Global_FNexusRetryExampleTest;

// Example: Performance monitoring with assertions
// This demonstrates the performance test macro with ArgusLens integration
NEXUS_PERF_TEST(FNexusPerformanceExampleTest, "Nexus.Performance.ContextExample", ETestPriority::Normal, 5.0f)
{
    // Example performance test that monitors metrics during execution
    
    if (!HAS_PERF_DATA(Context))
    {
        UE_LOG(LogTemp, Display, TEXT("Performance data not available (ArgusLens not running)"));
        return true;  // Pass if no perf data - this is OK for non-perf tests
    }
    
    // Use performance assertion helpers
    ASSERT_AVERAGE_FPS(Context, 30.0f);  // Minimum 30 FPS
    ASSERT_MAX_MEMORY(Context, 2048.0f);  // Maximum 2GB memory
    ASSERT_MAX_HITCHES(Context, 5);       // No more than 5 hitches
    
    UE_LOG(LogTemp, Display, TEXT("✅ Performance test passed!"));
    UE_LOG(LogTemp, Display, TEXT("  Average FPS: %.1f"), Context.PerformanceMetrics.AverageFPS);
    UE_LOG(LogTemp, Display, TEXT("  Peak Memory: %.0f MB"), Context.PerformanceMetrics.PeakMemoryMb);
    UE_LOG(LogTemp, Display, TEXT("  Hitches: %d"), Context.PerformanceMetrics.HitchCount);
    
    return true;
}
