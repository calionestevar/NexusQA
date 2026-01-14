#pragma once

#include "INexusEditorBridge.h"

class NEXUS_API FNexusEditorBridgeRegistry
{
public:
    static INexusEditorBridge& Get();

    static void Register(INexusEditorBridge* Bridge);
    static void Unregister(INexusEditorBridge* Bridge);
};
