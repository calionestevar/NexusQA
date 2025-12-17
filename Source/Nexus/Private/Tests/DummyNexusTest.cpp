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