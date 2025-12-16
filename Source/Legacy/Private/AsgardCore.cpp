// LEGACY: AsgardCore -- legacy commandlet-based test runner
//
// This module provides compatibility with the older Asgard commandlet and
// demonstrates use of Unreal's built-in AutomationTest framework. It is
// retained in the repository to show experience with engine-native
// testing patterns. The newer `Nexus` framework is the canonical CI/parallel
// orchestration path and should be preferred for automated runs.

#include "AsgardCore.h"
#include "Misc/AutomationTest.h"

void UAsgardCore::RunTestSuite(const FString& Filter)
{
    FAutomationTestRunner::RunTests(Filter);
}

void UAsgardCore::GenerateAIAssistedTest(const FString& Prompt, FString& OutTestCode)
{
    // Use AI API or placeholder (expand with OpenAI plugin later)
    OutTestCode = TEXT("IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGeneratedTest, \"Generated. " + Prompt + "\", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)");
}

bool UAsgardCore::ValidateTestEfficiency(float RuntimeSeconds, float MaxAllowed)
{
    return RuntimeSeconds <= MaxAllowed;
}