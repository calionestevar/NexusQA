#include "Modules/ModuleManager.h"

class FImGuiModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FImGuiModule, ImGui)
