#include "Modules/ModuleManager.h"

class FFargoEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FFargoEditorModule, FargoEditor)

void FFargoEditorModule::StartupModule()
{
	UE_LOG(LogFargoEditorModule, Warning, TEXT("🌐 FARGO EDITOR MODULE INITIALIZING"));
    
    FargoEditorBridge = MakeUnique<FFargoEditorBridge>();
    FNexusEditorBridgeRegistry::Register(FargoEditorBridge.Get());
	bFargoEditorModuleInitialized = true;

	UE_LOG(LogFargoEditorModule, Display, TEXT("✅ FARGO EDITOR MODULE ONLINE"));
}

void FFargoEditorModule::ShutdownModule()
{
    FNexusEditorBridgeRegistry::Unregister(FargoEditorBridge.Get());
    FargoEditorBridge.Reset();
}