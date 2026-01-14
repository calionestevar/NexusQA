#pragma once

#include "INexusEditorBridge.h"

class FFargoEditorBridge : public INexusEditorBridge
{
public:
    virtual bool IsEditorAvailable() const override;
    virtual bool EnsurePIEWorldActive(const FString& MapPath) override;
};
