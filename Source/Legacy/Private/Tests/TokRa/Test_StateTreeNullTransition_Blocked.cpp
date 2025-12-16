#pragma once
#include "Misc/AutomationTest.h"

// TOK'RA SYMBIOTE — PERMANENT RED BLOCKER
// This test intentionally fails in normal builds.
// If it ever passes → CI MUST FAIL → a known UE5 engine bug has returned.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRegStateTreeNullTransition, "UnrealQADemo.TokRa.StateTree_NullTransition_PermanentlyBlocked", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::CriticalPathFilter)
bool FRegStateTreeNullTransition::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("STATETREE NULL TRANSITION NULL POINTER — AI CORRUPTION — BLOCKED FOREVER"), false);
    return true;
}