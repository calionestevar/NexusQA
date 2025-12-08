#include "NexusDemo.h"
#include "NexusCore.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FNexusDemoModule, NexusDemo, "NexusDemo");

// Optional: auto-run Nexus when launched from command line
void FNexusDemoModule::StartupModule()
{
    if (IsRunningCommandlet() || FParse::Param(FCommandLine::Get(), TEXT("Nexus")))
    {
        UNexusCore::Execute(FCommandLine::GetArgv());
        // Quit after tests
        RequestEngineExit(TEXT("Nexus finished"));
    }
}