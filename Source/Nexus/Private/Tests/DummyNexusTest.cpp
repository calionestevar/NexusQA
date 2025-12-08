#include "NexusCore.h"

NEXUS_TEST(FNexusIsAliveTest, "Nexus.Smoke.DummyTest_ProvesFrameworkWorks")
{
    UE_LOG(LogTemp, Warning, TEXT("NEXUS IS ALIVE — THE REVOLUTION HAS BEGUN"));
    TestTrue(TEXT("42 is the answer to life"), 6 * 7 == 42);
    TestEqual(TEXT("Strings match"), FString(TEXT("Hello Nexus")), TEXT("Hello Nexus"));
    return true;
}

NEXUS_TEST(FNexusCanFailTest, "Nexus.Smoke.DummyTest_CanDetectFailure")
{
    TestTrue(TEXT("This test should fail on purpose"), false);
    return true;
}