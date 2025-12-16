#pragma once
#include "Nexus.h"
#include "CortexiphanInjector.h"

NEXUS_TEST(FCortexiphanChaosTest, "Chaos.Cortexiphan.FullRealityCollapse", ETestPriority::Critical)
{
    UCortexiphanInjector::InjectChaos(30.0f, 1.0f);
    return true; // Just lets chaos run — other tests feel it
}