#include "NexusEditorBridgeRegistry.h"

INexusEditorBridge* FNexusEditorBridgeRegistry::ActiveBridge = nullptr;

void FNexusEditorBridgeRegistry::Register(INexusEditorBridge* Bridge)
{
    ActiveBridge = Bridge;
}

void FNexusEditorBridgeRegistry::Unregister(INexusEditorBridgeRegistry* Bridge)
{
    if (ActiveBridge == Bridge)
    {
        ActiveBridge = nullptr;
    }
}

INexusEditorBridge* FNexusEditorBridgeRegistry::Get()
{
    return ActiveBridge;
}
