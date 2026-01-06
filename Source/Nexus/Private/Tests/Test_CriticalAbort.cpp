#include "NexusCore.h"

// A simple critical test that fails to exercise the abort sentinel logic
NEXUS_TEST_TAGGED(FNexusCriticalAbortTest, "Nexus.Critical.CriticalAbortTest", ETestPriority::Critical, {"Critical", "Abort"})
{
    UE_LOG(LogTemp, Error, TEXT("INTENTIONAL CRITICAL FAILURE — exercising abort sentinel."));
    return false; // force failure
}
