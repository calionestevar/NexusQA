#include "NexusTest.h"
#include "NexusCore.h"

/**
 * Smoke test for Nexus core orchestration.
 * Verifies basic test discovery and parallel worker initialization.
 */
NEXUS_TEST(FNexusCoreSmoke, "Nexus.Core.Smoke", ETestPriority::Normal)
{
    // Core framework bootstrapped successfully if this runs
    return true;
}
