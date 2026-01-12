#include "NexusEditorBridgeRegistry.h"
#include "NexusNullEditorBridge.h"

static TUniquePtr<INexusEditorBridge> ActiveBridge;

INexusEditorBridge& FNexusEditorBridgeRegistry::Get()
{
    if (!ActiveBridge)
    {
        ActiveBridge = MakeUnique<FNexusNullEditorBridge>();
    }
    return *ActiveBridge;
}

void FNexusEditorBridgeRegistry::Register(INexusEditorBridge* Bridge)
{
    if (Bridge)
    {
        ActiveBridge.Reset();
        ActiveBridge = TUniquePtr<INexusEditorBridge>(Bridge);
    }
}

void FNexusEditorBridgeRegistry::Unregister(INexusEditorBridge* Bridge)
{
    if (ActiveBridge.Get() == Bridge)
    {
        ActiveBridge = MakeUnique<FNexusNullEditorBridge>();
    }
}
