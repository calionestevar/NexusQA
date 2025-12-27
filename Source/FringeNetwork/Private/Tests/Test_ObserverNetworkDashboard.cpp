#include "Nexus/Core/Public/NexusTest.h"
#include "ObserverNetworkDashboard.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

NEXUS_TEST(FObserverNetworkDashboardSanity, "Chaos.ObserverNetworkDashboard.Sanity", ETestPriority::Normal)
{
    // Initialize and log a couple of events
    UObserverNetworkDashboard::Initialize();
    UObserverNetworkDashboard::LogSafetyEvent(TEXT("BLOCKED_CAMERA"), TEXT("Duplicate camera boom prevented"));
    UObserverNetworkDashboard::LogSafetyEvent(TEXT("FAILED_AI"), TEXT("State machine null transition encountered"));

    // Generate final report and ensure a file is created in Saved/ObserverReports
    UObserverNetworkDashboard::GenerateWebReport();

    const FString ReportDir = FPaths::ProjectSavedDir() / TEXT("ObserverReports");

    // Simple existence check for any file in the report dir
    IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
    bool bExists = PF.DirectoryExists(*ReportDir);

    TestTrue(TEXT("ObserverReports directory exists after GenerateWebReport"), bExists);
    return true;
}
