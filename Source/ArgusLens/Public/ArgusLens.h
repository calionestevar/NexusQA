#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ArgusLens.generated.h"

// Performance sample snapshot
USTRUCT(BlueprintType)
struct FPerformanceSample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float FrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float FPS = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float MemoryMb = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsHitch = false;

    UPROPERTY(BlueprintReadWrite)
    FString Timestamp;
};

// Performance threshold for auto-fail
USTRUCT(BlueprintType)
struct FPerformanceThreshold
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Thresholds")
    float MinFPS = 30.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Thresholds")
    float MaxFrameTimeMs = 33.0f; // ~30 FPS

    UPROPERTY(BlueprintReadWrite, Category = "Thresholds")
    float MaxMemoryMb = 2048.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Thresholds")
    float HitchThresholdMs = 100.0f; // Frames > 100ms considered hitches
};

/**
 * ArgusLens — Many-eyed Monitoring Performance Metrics Collector
 * Named after Argus Panoptes (Greek mythology): the many-eyed giant.
 * Collects live FPS, memory, hitch detection, network relevancy, and produces
 * artifacts for CI and recruiter demos.
 */
UCLASS()
class UArgusLens : public UObject
{
    GENERATED_BODY()

public:
    // Begin performance monitoring for duration
    UFUNCTION(BlueprintCallable, Category = "Performance|ArgusLens")
    static void StartPerformanceMonitoring(float DurationSeconds = 60.0f, bool bTrackNetRelevancy = true);

    // Stop monitoring and generate artifact
    UFUNCTION(BlueprintCallable, Category = "Performance|ArgusLens")
    static void StopPerformanceMonitoring();

    // Set performance thresholds for auto-fail conditions
    UFUNCTION(BlueprintCallable, Category = "Performance|ArgusLens")
    static void SetPerformanceThresholds(const FPerformanceThreshold& Thresholds);

    // Get average FPS over monitoring period
    UFUNCTION(BlueprintCallable, Category = "Performance|ArgusLens")
    static float GetAverageFPS();

    // Get peak memory usage
    UFUNCTION(BlueprintCallable, Category = "Performance|ArgusLens")
    static float GetPeakMemoryMb();

    // Get hitch count (frames exceeding threshold)
    UFUNCTION(BlueprintCallable, Category = "Performance|ArgusLens")
    static int32 GetHitchCount();

    // Export performance report to JSON + CSV + HTML
    UFUNCTION(BlueprintCallable, Category = "Performance|ArgusLens")
    static void ExportPerformanceArtifact(const FString& OutputPath = TEXT(""));

    // Check if run passed all thresholds
    UFUNCTION(BlueprintCallable, Category = "Performance|ArgusLens")
    static bool DidPassPerformanceGates();

    // Get current performance snapshot (for assertion context)
    static FPerformanceSample GetCurrentPerformanceSnapshot();
};
