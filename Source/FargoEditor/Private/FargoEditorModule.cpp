#include "FargoEditorBridge.h"
#include "INexusEditorBridge.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogFargoEditor, Log, All);

class FFargoEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TUniquePtr<FFargoEditorBridge> FargoEditorBridge;
};

IMPLEMENT_MODULE(FFargoEditorModule, FargoEditor)

void FFargoEditorModule::StartupModule()
{
    UE_LOG(LogFargoEditor, Warning, TEXT("🌐 FARGO EDITOR MODULE INITIALIZING"));

    FargoEditorBridge = MakeUnique<FFargoEditorBridge>();
    RegisterNexusEditorBridge(FargoEditorBridge.Get());

    UE_LOG(LogFargoEditor, Display, TEXT("✅ FARGO EDITOR MODULE ONLINE"));
}

void FFargoEditorModule::ShutdownModule()
{
    UnregisterNexusEditorBridge(FargoEditorBridge.Get());
    FargoEditorBridge.Reset();
}
