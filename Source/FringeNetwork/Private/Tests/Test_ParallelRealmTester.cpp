#include "NexusTest.h"
#include "FringeNetwork.h"

// This test is marked OnlineOnly: it makes HTTP HEAD requests. On CI, the network
// environment may vary; testers can skip OnlineOnly tests by passing flags to the
// Nexus harness.
NEXUS_TEST(FParallelRealmTesterOnline, "FringeNetwork.ParallelRealmTester.OnlineCheck", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
    UFringeNetwork* FG = NewObject<UFringeNetwork>();
    if (!FG)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to construct UFringeNetwork for test"));
        return false;
    }

    TArray<FString> Regions;
    // Use a couple of well-known stable hosts for a basic reachability probe.
    Regions.Add(TEXT("https://www.example.com/"));
    Regions.Add(TEXT("https://www.google.com/"));

    // The method internally waits up to 5s for responses and asserts 90%+ success.
    FG->TestParallelRealms(Regions);

    return true;
}
