#include "Nexus/Core/Public/NexusTest.h"
#include "ArgusLens.h"

/**
 * Sample performance monitoring test.
 * Demonstrates ArgusLens metrics collection.
 */
NEXUS_TEST(FArgusLensSmoke, "ArgusLens.Performance.Smoke", ETestPriority::Normal)
{
    // Start monitoring for 5 seconds
    UArgusLens::StartPerformanceMonitoring(5.0f, false);
    return true;
}
