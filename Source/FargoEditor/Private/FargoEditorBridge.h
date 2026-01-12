#pragma once

#include "INexusEditorBridge.h"

class FFargoEditorBridge : public INexusEditorBridge
{
public:
    virtual bool EnsurePIEWorldActive(const FString& MapPath) override;
};
