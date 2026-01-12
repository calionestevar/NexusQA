#pragma once

#include "INexusEditorBridge.h"

class FNexusNullEditorBridge : public INexusEditorBridge
{
public:
    virtual bool EnsurePIEWorldActive(const FString& /*MapPath*/) override
    {
        return false;
    }

    virtual bool IsEditorAvailable() const override
    {
        return false;
    }
};
