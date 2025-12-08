#include "PalantirCapture.h"
#include "Engine/Engine.h"
#include "HighResScreenshot.h"
#include "PalantirObserver.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Async/Async.h"

void FPalantirCapture::TakeScreenshotOnFailure(const FString& TestName)
{
    if (GEngine && GEngine->GameViewport)
    {
        FString SafeName = TestName.Replace(TEXT(" "), TEXT("_"));
        const FString BaseName = SafeName + TEXT("_FAIL");
        // Request screenshot (asynchronous). File will be saved under Saved/Screenshots/<Platform>/
        FScreenshotRequest::RequestScreenshot(BaseName, false, true);

        // Wait for the screenshot file to appear (best-effort) on a background thread.
        const FString SavedDir = FPaths::ProjectSavedDir();
        const FString ScreenshotsDir = FPaths::Combine(SavedDir, TEXT("Screenshots"));
        const double TimeoutSeconds = 8.0;
        const double PollInterval = 0.25;
        FString CapturedBaseName = BaseName; // copy for thread
        FString CapturedTestName = TestName;

        // Launch a background task to poll for the asynchronous screenshot and register artifacts.
        Async(EAsyncExecution::ThreadPool, [CapturedBaseName, CapturedTestName, ScreenshotsDir, SavedDir, TimeoutSeconds, PollInterval]() {
            double Elapsed = 0.0;
            FString FoundPath;
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

            while (Elapsed < TimeoutSeconds)
            {
                TArray<FString> FoundFiles;
                PlatformFile.FindFilesRecursively(FoundFiles, *ScreenshotsDir, *FString::Printf(TEXT("%s.png"), *CapturedBaseName));
                if (FoundFiles.Num() > 0)
                {
                    FoundPath = FoundFiles[0];
                    break;
                }
                FPlatformProcess::Sleep(PollInterval);
                Elapsed += PollInterval;
            }

            if (!FoundPath.IsEmpty())
            {
                FPalantirObserver::RegisterArtifact(CapturedTestName, FoundPath);
                UE_LOG(LogTemp, Error, TEXT("PALANTÍR CAPTURED FAILURE: %s (found %s)"), *CapturedBaseName, *FoundPath);
            }
            else
            {
                const FString ExpectedPath = FPaths::Combine(SavedDir, TEXT("Screenshots"), CapturedBaseName + TEXT(".png"));
                FPalantirObserver::RegisterArtifact(CapturedTestName, ExpectedPath);
                UE_LOG(LogTemp, Warning, TEXT("PALANTÍR requested screenshot but file not found within timeout; registered expected path %s"), *ExpectedPath);
            }

            // Also register the most recent engine log file for context
            const FString LogsDir = FPaths::Combine(SavedDir, TEXT("Logs"));
            if (PlatformFile.DirectoryExists(*LogsDir))
            {
                TArray<FString> LogFiles;
                PlatformFile.FindFilesRecursively(LogFiles, *LogsDir, TEXT("*.log"));
                if (LogFiles.Num() > 0)
                {
                    // Choose the newest by timestamp
                    LogFiles.Sort([&](const FString& A, const FString& B) {
                        return PlatformFile.GetTimeStamp(*A) > PlatformFile.GetTimeStamp(*B);
                    });
                    FPalantirObserver::RegisterArtifact(CapturedTestName, LogFiles[0]);
                    UE_LOG(LogTemp, Display, TEXT("PALANTÍR registered log for %s -> %s"), *CapturedTestName, *LogFiles[0]);
                }
            }
        });
    }
}