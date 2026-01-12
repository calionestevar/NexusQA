#pragma once

#include "INexusEditorBridge.h"

class FNexusEditorBridgeRegistry
{
public:
    static void Register(INexusEditorBridge* Bridge);
    static void Unregister(INexusEditorBridge* Bridge);
    static INexusEditorBridge* Get();

private:
    static INexusEditorBridge* ActiveBridge;
};
