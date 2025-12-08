#pragma once
#include "CoreMinimal.h"

class PALANTIRCAPTURE_API FPalantirCapture
{
public:
    static void TakeScreenshotOnFailure(const FString& TestName);
};