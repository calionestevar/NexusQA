#include "Modules/ModuleManager.h"

class FFargoEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FFargoEditorModule, FargoEditor)