#include "FargoEditorBridge.h"
#include "Editor/EditorEngine.h"
#include "PlayInEditorDataTypes.h"

bool FFargoEditorBridge::IsEditorAvailable() const
{
#if WITH_EDITOR
    return GEditor != nullptr;
#else
    return false;
#endif
}

bool FFargoEditorBridge::EnsurePIEWorldActive(const FString& /*MapPath*/)
{
    if (!GEditor)
    {
        return false;
    }

    FRequestPlaySessionParams Params;
    Params.SessionPreviewTypeOverride = EPlaySessionPreviewType::NoPreview;

    GEditor->RequestPlaySession(Params);
    return true;
}
