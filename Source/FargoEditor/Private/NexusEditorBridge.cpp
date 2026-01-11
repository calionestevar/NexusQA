#include "NexusEditorBridge.h"

#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

bool FNexusEditorBridge::EnsurePIEWorldActive(const FString& MapPath)
{
    if (!GEditor)
    {
        UE_LOG(LogTemp, Error, TEXT("GEditor is null"));
        return false;
    }

    if (GEditor->PlayWorld)
    {
        // PIE already running
        return true;
    }   

    FRequestPlaySessionParams Params;

    if (!MapPath.IsEmpty())
    {
        Params.Map = MapPath;
    }

    GEditor->RequestPlaySession(Params);
    return true;
}
