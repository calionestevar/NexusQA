#pragma once

#include "CoreMinimal.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"

inline bool Nexus_IsRunningCommandlet()
{
    return IsRunningCommandlet();
}

inline bool Nexus_IsUnattended()
{
    return FApp::IsUnattended();
}

inline bool Nexus_IsAutomationTesting()
{
    return GIsAutomationTesting;
}

inline bool Nexus_IsCIEnvironment()
{
    return Nexus_IsRunningCommandlet()
        || Nexus_IsUnattended()
        || Nexus_IsAutomationTesting();
}

inline bool Nexus_ShouldAvoidEditorFeatures()
{
    return Nexus_IsCIEnvironment();
}
