#pragma once
#include "Misc/AutomationTest.h"

// TOK'RA SYMBIOTE — PERMANENT RED BLOCKER
// This test intentionally fails in normal builds.
// If it ever passes → CI MUST FAIL → a known UE5 engine bug has returned.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRegDuplicateCameraBoom, "UnrealQADemo.TokRa.DuplicateCameraBoom_PermanentlyBlocked", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::CriticalPathFilter)
bool FRegDuplicateCameraBoom::RunTest(const FString& Parameters)
{
    AUnrealQADemoCharacter* C = NewObject<AUnrealQADemoCharacter>();
    TArray<USpringArmComponent*> Booms; C->GetComponents(Booms);
    TestTrue(TEXT("DUPLICATE CAMERABOOM — C++ VS BP CONFLICT — BLOCKED FOREVER"), Booms.Num() <= 1);
    return true;
}