#include "Nexus/Core/Public/NexusTest.h"
#include "ObserverNetworkDashboard.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

NEXUS_TEST(FObserverNetworkDashboardSanity, "Chaos.ObserverNetworkDashboard.Sanity", ETestPriority::Normal)
{
    // Initialize and log a couple of events
    UObserverNetworkDashboard::Initialize();
    UObserverNetworkDashboard::LogSafetyEvent(TEXT("BLOCKED_CAMERA"), TEXT("Duplicate camera boom prevented"));
    UObserverNetworkDashboard::LogSafetyEvent(TEXT("BLOCKED_RIGGING"), TEXT("Invalid bone detected in skeleton"));
    
    // Log additional safety events
    UObserverNetworkDashboard::LogSafetyEvent(TEXT("FAILED_AI"), TEXT("State machine null transition encountered"));

    // Generate final report and ensure a file is created in Saved/ObserverReports
    UObserverNetworkDashboard::GenerateWebReport();

    const FString ReportDir = FPaths::ProjectSavedDir() / TEXT("ObserverReports");

    // Simple existence check for the report directory
    IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
    bool bExists = PF.DirectoryExists(*ReportDir);

    if (!bExists)
    {
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: ObserverReports directory does not exist after GenerateWebReport"));
        return false;  // Fail the test
    }

    UE_LOG(LogTemp, Display, TEXT("Observer Dashboard sanity test: All checks passed"));
    return true;
}
