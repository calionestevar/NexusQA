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
    UWorld* World = GWorld ? GWorld : (GEngine && GEngine->GetCurrentPlayWorld() ? GEngine->GetCurrentPlayWorld() : nullptr);
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
    UE_LOG(LogArgusLens, Display, TEXT("ArgusLens: Thresholds set - MinFPS=%.0f, MaxMemory=%.0fMB, HitchThreshold=%.0fms"),
        Thresholds.MinFPS, Thresholds.MaxMemoryMb, Thresholds.HitchThresholdMs);
}

float UArgusLens::GetAverageFPS()
{
    if (GPerformanceSamples.Num() == 0) return 0.0f;

    double TotalFPS = 0.0;
    for (const FPerformanceSample& Sample : GPerformanceSamples)
    {
        TotalFPS += Sample.FPS;
    }
    return static_cast<float>(TotalFPS / GPerformanceSamples.Num());
}

float UArgusLens::GetPeakMemoryMb()
{
    return GPeakMemory;
}

int32 UArgusLens::GetHitchCount()
{
    return GTotalHitches;
}

bool UArgusLens::DidPassPerformanceGates()
{
    if (GPerformanceSamples.Num() == 0) return true;

    float AvgFPS = GetAverageFPS();
    return AvgFPS >= GPerformanceThresholds.MinFPS && GPeakMemory <= GPerformanceThresholds.MaxMemoryMb;
}

void UArgusLens::ExportPerformanceArtifact(const FString& OutputPath)
{
    const float AvgFPS = GetAverageFPS();
    const float PeakMemory = GetPeakMemoryMb();
    const int32 HitchCount = GetHitchCount();
    const bool bPassed = DidPassPerformanceGates();

    // Build JSON report
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("timestamp"), FDateTime::Now().ToString());
    Root->SetNumberField(TEXT("averageFPS"), AvgFPS);
    Root->SetNumberField(TEXT("peakMemoryMb"), PeakMemory);
    Root->SetNumberField(TEXT("hitchCount"), HitchCount);
    Root->SetBoolField(TEXT("passedThresholds"), bPassed);
    Root->SetNumberField(TEXT("sampleCount"), GPerformanceSamples.Num());

    // Determine output path
    FString OutputFile = OutputPath.IsEmpty() ? 
        FPaths::ProjectSavedDir() / TEXT("NexusReports/ArgusLensPerformance.json") : OutputPath;
    
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(OutputFile));

    // Write JSON
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
    FFileHelper::SaveStringToFile(JsonString, *OutputFile);

    UE_LOG(LogArgusLens, Display, TEXT("ArgusLens: Exported performance artifact to %s"), *OutputFile);
    UE_LOG(LogArgusLens, Display, TEXT("  AvgFPS: %.1f | PeakMem: %.0fMB | Hitches: %d | Passed: %s"),
        AvgFPS, PeakMemory, HitchCount, bPassed ? TEXT("YES") : TEXT("NO"));
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
