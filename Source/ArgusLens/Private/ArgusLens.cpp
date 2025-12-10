#include "ArgusLens.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformMemory.h"
#include "Json.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "PalantirTrace.h"

DEFINE_LOG_CATEGORY_STATIC(LogArgusLens, Display, All);

// Global performance monitoring state
static TArray<FPerformanceSample> GPerformanceSamples;
static FPerformanceThreshold GPerformanceThresholds;
static FTimerHandle GPerformanceMonitorHandle;
static int32 GTotalHitches = 0;
static float GPeakMemory = 0.0f;

void UArgusLens::StartPerformanceMonitoring(float DurationSeconds, bool bTrackNetRelevancy)
{
    UE_LOG(LogArgusLens, Display, TEXT("ArgusLens: Starting performance monitoring for %.0f seconds"), DurationSeconds);
    
    // Add breadcrumb if trace is active
    if (!FPalantirTrace::GetCurrentTraceID().IsEmpty())
    {
        PALANTIR_BREADCRUMB(TEXT("ArgusLens"), TEXT("Performance monitoring started"));
    }

    GPerformanceSamples.Empty();
    GTotalHitches = 0;
    GPeakMemory = 0.0f;

    UWorld* World = GEngine ? GEngine->GetGameWorld() : nullptr;
    if (!World)
    {
        UE_LOG(LogArgusLens, Warning, TEXT("ArgusLens: No world context - skipping monitoring"));
        return;
    }

    // Sample every 100ms
    World->GetTimerManager().SetTimer(GPerformanceMonitorHandle, FTimerDelegate::CreateLambda([]()
    {
        FPerformanceSample Sample;
        Sample.Timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));

        // Sample FPS and frame time
        if (GEngine)
        {
            float DeltaTime = GEngine->GetMaxTickRate(0.0f, false);
            Sample.FrameTimeMs = DeltaTime * 1000.0f;
            Sample.FPS = (DeltaTime > 0.0f) ? 1.0f / DeltaTime : 60.0f; // Default assumption
        }

        // Sample memory
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        Sample.MemoryMb = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);

        // Check if frame is a hitch
        Sample.bIsHitch = Sample.FrameTimeMs > GPerformanceThresholds.HitchThresholdMs;
        if (Sample.bIsHitch)
        {
            GTotalHitches++;
            UE_LOG(LogArgusLens, Warning, TEXT("ArgusLens: Hitch detected - Frame time: %.1fms"), Sample.FrameTimeMs);
        }

        // Track peak memory
        if (Sample.MemoryMb > GPeakMemory)
        {
            GPeakMemory = Sample.MemoryMb;
        }

        GPerformanceSamples.Add(Sample);
    }), 0.1f, true);

    // Stop monitoring after duration
    FTimerHandle StopHandle;
    World->GetTimerManager().SetTimer(StopHandle, FTimerDelegate::CreateLambda([World]()
    {
        if (World)
        {
            World->GetTimerManager().ClearTimer(GPerformanceMonitorHandle);
        }
        {
            FScopeLock Lock(&GPerformanceMutex);
            bMonitoring = false;
        }
        ArgusLog(TEXT("PERFORMANCE MONITORING STOPPED"));
    }), DurationSeconds, false);
}

void UArgusLens::StopPerformanceMonitoring()
{
    UWorld* World = GEngine->GetFirstLocalPlayerController() ? GEngine->GetFirstLocalPlayerController()->GetWorld() : nullptr;
    if (World)
    {
        World->GetTimerManager().ClearTimer(GPerformanceMonitorHandle);
    }
    {
        FScopeLock Lock(&GPerformanceMutex);
        bMonitoring = false;
    }
    ArgusLog(TEXT("PERFORMANCE MONITORING STOPPED"));
}

void UArgusLens::SetPerformanceThresholds(const FPerformanceThreshold& Thresholds)
{
    GPerformanceThresholds = Thresholds;
    ArgusLog(FString::Printf(TEXT("Performance thresholds set: MinFPS=%.0f, MaxMemory=%.0f MB, HitchThreshold=%.0f ms"),
        Thresholds.MinFPS, Thresholds.MaxMemoryMb, Thresholds.HitchThresholdMs));
}

float UArgusLens::GetAverageFPS()
{
    FScopeLock Lock(&GPerformanceMutex);
    if (GPerformanceSamples.Num() == 0) return 0.0f;

    double TotalFPS = 0.0;
    for (const auto& Sample : GPerformanceSamples)
    {
        TotalFPS += Sample.FPS;
    }
    return (float)(TotalFPS / GPerformanceSamples.Num());
}

float UArgusLens::GetPeakMemoryMb()
{
    FScopeLock Lock(&GPerformanceMutex);
    return GPeakMemory;
}

int32 UArgusLens::GetHitchCount()
{
    FScopeLock Lock(&GPerformanceMutex);
    return GTotalHitches;
}

bool UArgusLens::DidPassPerformanceGates()
{
    FScopeLock Lock(&GPerformanceMutex);
    if (GPerformanceSamples.Num() == 0) return true;

    float AvgFPS = GetAverageFPS();
    bool bFPSPass = AvgFPS >= GPerformanceThresholds.MinFPS;
    bool bMemoryPass = GPeakMemory <= GPerformanceThresholds.MaxMemoryMb;

    return bFPSPass && bMemoryPass;
}

void UArgusLens::ExportPerformanceArtifact(const FString& OutputPath)
{
    TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);

    Root->SetStringField(TEXT("timestamp"), FDateTime::Now().ToString());
    Root->SetStringField(TEXT("title"), TEXT("ARGUSLENS PERFORMANCE METRICS REPORT"));

    float AvgFPS = GetAverageFPS();
    float PeakMemory = GetPeakMemoryMb();
    int32 HitchCount = GetHitchCount();
    bool bPassed = DidPassPerformanceGates();

    // Summary
    Root->SetNumberField(TEXT("averageFPS"), AvgFPS);
    Root->SetNumberField(TEXT("peakMemoryMb"), PeakMemory);
    Root->SetNumberField(TEXT("hitchCount"), HitchCount);
    Root->SetBoolField(TEXT("passedThresholds"), bPassed);

    // Thresholds
    TSharedPtr<FJsonObject> ThresholdsObj = MakeShareable(new FJsonObject);
    ThresholdsObj->SetNumberField(TEXT("minFPS"), GPerformanceThresholds.MinFPS);
    ThresholdsObj->SetNumberField(TEXT("maxMemoryMb"), GPerformanceThresholds.MaxMemoryMb);
    ThresholdsObj->SetNumberField(TEXT("hitchThresholdMs"), GPerformanceThresholds.HitchThresholdMs);
    Root->SetObjectField(TEXT("thresholds"), ThresholdsObj);

    // Sample of performance data (every 10th sample to reduce file size)
    TArray<TSharedPtr<FJsonValue>> SamplesArray;
    {
        FScopeLock Lock(&GPerformanceMutex);
        for (int32 i = 0; i < GPerformanceSamples.Num(); i += 10)
        {
            const auto& Sample = GPerformanceSamples[i];
            TSharedPtr<FJsonObject> SampleObj = MakeShareable(new FJsonObject);
            SampleObj->SetStringField(TEXT("time"), Sample.Timestamp);
            SampleObj->SetNumberField(TEXT("fps"), Sample.FPS);
            SampleObj->SetNumberField(TEXT("frameTimeMs"), Sample.FrameTimeMs);
            SampleObj->SetNumberField(TEXT("memoryMb"), Sample.MemoryMb);
            SampleObj->SetBoolField(TEXT("isHitch"), Sample.bIsHitch);
            SamplesArray.Add(MakeShareable(new FJsonValueObject(SampleObj)));
        }
    }
    Root->SetArrayField(TEXT("samples"), SamplesArray);

    // Serialize to JSON
    FString OutputFile = OutputPath;
    if (OutputFile.IsEmpty())
    {
        OutputFile = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("NexusReports/ArgusLensPerformance.json"));
    }
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(*OutputFile));

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

    if (FFileHelper::SaveStringToFile(JsonString, *OutputFile))
    {
        ArgusLog(FString::Printf(TEXT("PERFORMANCE ARTIFACT EXPORTED -> %s"), *OutputFile));
    }
    else
    {
        ArgusLog(FString::Printf(TEXT("FAILED TO WRITE PERFORMANCE ARTIFACT -> %s"), *OutputFile));
    }

    // Generate simple HTML dashboard
    FString HtmlContent = FString::Printf(TEXT(
        R"(<!DOCTYPE html>
<html>
<head><title>Performance Report</title></head>
<body style="font-family: Arial; background: #f5f5f5; margin: 20px;">
<h1>ARGUSLENS Performance Report</h1>
<p>Generated: %s</p>
<h2>Summary</h2>
<ul>
<li><strong>Average FPS:</strong> %.1f</li>
<li><strong>Peak Memory:</strong> %.0f MB</li>
<li><strong>Hitch Count:</strong> %d</li>
<li><strong>Passed Thresholds:</strong> %s</li>
</ul>
</body></html>)"
    ), *FDateTime::Now().ToString(), AvgFPS, PeakMemory, HitchCount, bPassed ? TEXT("YES") : TEXT("NO"));

    FString HtmlPath = FPaths::Combine(FPaths::GetPath(*OutputFile), TEXT("ArgusLensPerformance.html"));
    FFileHelper::SaveStringToFile(HtmlContent, *HtmlPath);
}

FPerformanceSample UArgusLens::GetCurrentPerformanceSnapshot()
{
    FPerformanceSample Snapshot;
    
    if (GEngine)
    {
        float DeltaTime = GEngine->GetDeltaSeconds();
        Snapshot.FrameTimeMs = DeltaTime * 1000.0f;
        Snapshot.FPS = (DeltaTime > 0.0f) ? 1.0f / DeltaTime : 0.0f;
    }
    
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Snapshot.MemoryMb = (float)MemStats.UsedPhysical / (1024.0f * 1024.0f);
    Snapshot.bIsHitch = Snapshot.FrameTimeMs > GPerformanceThresholds.HitchThresholdMs;
    Snapshot.Timestamp = FDateTime::Now().ToString();
    
    return Snapshot;
}
