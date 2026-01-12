#pragma once

#include "CoreMinimal.h"

class INexusEditorBridge
{
public:
    virtual ~INexusEditorBridge() = default;

    virtual bool EnsurePIEWorldActive(const FString& MapPath) = 0;
    virtual bool IsEditorAvailable() const = 0;
};
