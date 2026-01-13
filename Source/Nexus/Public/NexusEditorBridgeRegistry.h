#pragma once

#include "INexusEditorBridge.h"

class FNexusEditorBridgeRegistry
{
public:
    static INexusEditorBridge& Get();

    static void Register(INexusEditorBridge* Bridge);
    static void Unregister(INexusEditorBridge* Bridge);
};
