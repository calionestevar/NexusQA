#pragma once
#include "Commandlets/Commandlet.h"
#include "AutomationControllerModule.h"
#include "IAutomationControllerManager.h"
#include "AsgardCommandlet.generated.h"

/**
 * Legacy commandlet demonstrating integration with Unreal's built-in automation framework.
 * This shows competence with engine-native testing patterns.
 * 
 * For enhanced features (parallel execution, distributed tracing, fail-fast critical tests),
 * use the Nexus test framework instead.
 */
UCLASS()
class LEGACY_API UAsgardCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    virtual int32 Main(const FString& Params) override;

private:
    void ExportTestResults(const FString& OutputPath, IAutomationControllerManagerRef Controller);
};