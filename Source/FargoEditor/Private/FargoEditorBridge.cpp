#include "FargoEditorBridge.h"
#include "Editor.h"
#include "PlayInEditorDataTypes.h"

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
