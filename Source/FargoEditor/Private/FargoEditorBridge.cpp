#include "FargoEditorBridge.h"

#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "UnrealEdGlobals.h"

bool FFargoEditorBridge::EnsurePIEWorldActive(const FString& MapPath)
{
    if (!GEditor || !GEngine)
    {
        return false;
    }

    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (Context.World() && Context.World()->IsGameWorld())
        {
            return true;
        }
    }

    if (MapPath.IsEmpty())
    {
        return false;
    }

    FRequestPlaySessionParams Params;
    Params.MapToLoad = MapPath;

    GEditor->RequestPlaySession(Params);
    return true;
}
