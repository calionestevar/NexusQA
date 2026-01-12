#include "NexusEditorBridge.h"

#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

bool FNexusEditorBridge::EnsurePIEWorldActive(const FString& MapPath)
{
    if (!GEditor || !GEngine)
    {
        return false;
    }

    // Already running?
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
    Params.bSimulateInEditor = false;
    Params.bPlayInEditorFloating = false;
    Params.SessionPreviewTypeOverride = EPlaySessionPreviewType::None;

    GEditor->RequestPlaySession(Params);

    // Legacy wait loop (to be replaced later)
    double Start = FPlatformTime::Seconds();
    while (FPlatformTime::Seconds() - Start < 5.0)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.World() && Context.World()->IsGameWorld())
            {
                return true;
            }
        }
        FPlatformProcess::Sleep(0.1f);
    }

    return false;
}
