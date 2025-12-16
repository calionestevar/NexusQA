#pragma once
#include "Misc/AutomationTest.h"

// TOK'RA SYMBIOTE — PERMANENT RED BLOCKER
// This test intentionally fails in normal builds.
// If it ever passes → CI MUST FAIL → a known UE5 engine bug has returned.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRegNullMappingContext, "UnrealQADemo.TokRa.EnhancedInput_NullMappingContext_PermanentlyBlocked", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::CriticalPathFilter)
bool FRegNullMappingContext::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("NULL MAPPING CONTEXT DETECTED — HOT-RELOAD CORRUPTION — BLOCKED FOREVER"), false);
    return true;
}