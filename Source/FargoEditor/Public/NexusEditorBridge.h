#pragma once

#include "CoreMinimal.h"

class FNexusEditorBridge
{
public:
    static bool EnsurePIEWorldActive(const FString& MapPath);
};
