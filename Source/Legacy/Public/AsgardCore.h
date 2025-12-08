#pragma once
#include "CoreMinimal.h"
#include "AsgardCore.generated.h"

UCLASS()
class UAsgardCore : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "AsgardCore")
    static void RunTestSuite(const FString& Filter = TEXT("*"));

    UFUNCTION(BlueprintCallable, Category = "AsgardCore")
    static void GenerateAI AssistedTest(const FString& Prompt, FString& OutTestCode);

    UFUNCTION(BlueprintCallable, Category = "AsgardCore")
    static bool ValidateTestEfficiency(float RuntimeSeconds, float MaxAllowed = 45.0f);
};